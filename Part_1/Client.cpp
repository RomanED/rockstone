
#include "Client.h"

#include "Log.h"
#include "LoginData.h"

#include <stdexcept>
#include <string>

Client::Client(IO *io) : io(io)
{
	this->requests = std::make_shared<Requests>(this);
}

Client::~Client() = default;

void Client::handle_error(std::string_view message, std::source_location loc) const
{
    logger->error(std::format("[{}:{}] Client error: {}", 
        loc.file_name(), loc.line(), message));
    this->disconnect();
}

auto Client::get_ip() const -> IP
{
	return this->io->get_ip();
}

void Client::disconnect() const
{
	this->io->stop();
}

void Client::send(const server::Packet &packet) const
{
	io->write(packet);
}

void Client::on_event(const ClientEvent &event)
{
	// Пример обработки событий клиента
	switch (event.get_type())
	{
		case ClientEvent::Type::DISCONNECT:
			this->disconnect();
			break;
		default:
			logger->warning("Unhandled event type {}", event.get_type());
	}
}

// Обновлённый обработчик входящих пакетов
void Client::on_packet(const server::Packet &packet)
{
    try
    {
        switch (packet.get_type())
        {
            case server::PacketType::PARAMS_SET:
                this->params_set(packet);
                break;
            case server::PacketType::BUY:
                this->buy(packet);
                break;
            case server::PacketType::LOGIN:
                this->login(packet);
                break;
            default:
                logger->warning("Unhandled packet type {}", static_cast<int>(packet.get_type()));
        }
    }
    catch (...)
    {
        handle_error("Critical error in packet processing");
    }
}

auto Client::parse_login_packet(const server::Packet& packet) 
    -> std::expected<LoginData, std::string_view>
{
    using namespace std::string_view_literals;
    
    if (packet.fields_count() < 4)
    {
        return std::unexpected("Insufficient fields"sv);
    }

    try
    {
        return LoginData
        {
            .net_id = packet.L(0),
            .net_type = packet.B(1),
            .auth_key = packet.S(3)
        };
    }
    catch (const std::out_of_range&)
    {
        return std::unexpected("Field index out of range"sv);
    }
    catch (const std::bad_variant_access&)
    {
        return std::unexpected("Field type mismatch"sv);
    }
}

void Client::handle_error(std::string_view message, std::source_location loc = std::source_location::current())
{
    logger->error("[{}:{}] {}",
        loc.file_name(), loc.line(), message);
    disconnect();
}

void Client::params_set(const server::Packet &packet)
{

    if (state != ClientState::LOGGED_IN) [[unlikely]]
    {
        logger->debug("Params_set called before login");
        return;
    }

    try
    {
        std::string params = packet.S(0);
        if (!params.empty())
        {
            player->params->set(params);
            logger->info(std::format("Params updated for {}", player->id));
        }
    }
    catch (const std::exception& e)
    {
        handle_error(std::format("Params_set error: {}", e.what()));
    }
}

void Client::buy(const server::Packet &packet)
{
    if (state != ClientState::LOGGED_IN) [[unlikely]]
    {
        logger->debug("Buy attempt before login");
        return;
    }

    try {
        uint32_t item_id = packet.I(0);
        
        // Расширенная валидация
        if (item_id == 0)
        {
            throw std::invalid_argument("Invalid item ID");
        }

        if (player->balance->can_afford(item_id))
        {
            player->balance->deduct(item_id);
            player->inventory->add(item_id);
            logger->info(std::format("Item {} bought by {}", item_id, player->id));
        } 
        else
        {
            logger->warning(std::format("Client {} can't afford item {}", player->id, item_id));
        }
    }
    catch (const std::exception& e)
    {
        handle_error(std::format("Buy error: {}", e.what()));
    }
}

void Client::login(const server::Packet &packet)
{
   // Проверка состояния
    if (state != ClientState::CONNECTED) [[unlikely]]
    {
        logger->warning("Duplicate login attempt from {}", get_ip());
        send(server::Login(server::Login::Status::FAILED));
        return;
    }

    // Парсинг с  обработкой ошибок
    auto login_data = parse_login_packet(packet);
    if (!login_data)
    {
        handle_error(std::format("Login error: {}", login_data.error()));
        send(server::Login(server::Login::Status::FAILED));
        return;
    }

    // Валидация бизнес-логики
    if (login_data->net_type >= NetType::MAX_TYPE) [[unlikely]] {
        handle_error(std::format("Invalid net_type: {}", login_data->net_type));
        send(server::Login(server::Login::Status::FAILED));
        return;
    }

    // Асинхронная аутентификация с безопасным владением данными
    state = ClientState::AUTHENTICATING;
    auto auth_data = std::make_shared<LoginData>(std::move(*login_data));

    this->requests->add(auth_data.get(), [this, auth_data](const auto& players) 
    {
        // Проверка актуальности состояния
        if (state != ClientState::AUTHENTICATING) [[unlikely]]
        {
            logger->debug("Auth aborted for {}", auth_data->net_id);
            return;
        }

        // Обработка результата
        if (players.empty() || !players[0])
        {
            handle_error("Player data not loaded");
            state = ClientState::CONNECTED;
            return;
        }

        player = players[0];
        state = ClientState::LOGGED_IN;
        logger->info(std::format("Client {} logged in", player->id));
        send(server::Login(server::Login::Status::OK));
    });
}

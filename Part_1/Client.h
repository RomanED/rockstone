
#pragma once

#include "IO.h"
#include "Requests.h"
#include "Player.h"
#include "ClientEvent.h"

#include "server/Packet.h"

#include <memory>
#include <expected>
#include <source_location>
#include <format>
#include <string_view>

enum class ClientState
{
    CONNECTED,
    AUTHENTICATING,
    LOGGED_IN
};

class Client final
{
public:
	explicit Client(IO *io);
	~Client();

	void disconnect() const;
	void send(const server::Packet &packet) const;
	void on_event(const ClientEvent &event);

private:
	IO *io;
	std::shared_ptr<Requests> requests;
	Player *player = nullptr;
	ClientState state = ClientState::CONNECTED;

	[[nodiscard]] auto get_ip() const -> IP;

	void on_packet(const server::Packet &packet);
        
        // Парсинг с обработкой ошибок
        auto parse_login(const server::Packet& packet) -> std::expected<LoginData, std::string>;
        void handle_error(std::string_view message, 
                          std::source_location loc = std::source_location::current()) const;

        // Обработчики пакетов
        void params_set(const server::Packet &packet);
        void buy(const server::Packet &packet);
        void login(const server::Packet &packet);

        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;
};

# Задача 3

Дано: набор данных в таблице players со структурой полей:
 players(id INT, name VARCHAR, login_time DATETIME, device INT)
Задание: сформулировать SQL-запросы для следующих случаев:
1. Получить 5 самых активных устройств – то есть пять значений поля device, с которых произошло наибольшее количество логинов пользователей (упорядочить по количеству логинов).

2. Получить среднее число логинов в день за последние 7 дней (в расчете от текущей даты).


## Структура таблицы
```sql
players(
    id INT,
    name VARCHAR,
    login_time DATETIME,
    device INT
)
```

---

## Задание 1: Топ-5 самых активных устройств
Получить 5 значений поля `device` с наибольшим количеством логинов.

### Решение:
```sql
SELECT device, COUNT(*) AS login_count
FROM players
GROUP BY device
ORDER BY login_count DESC
LIMIT 5;
```

**Пояснение:**
- `GROUP BY device` - группировка по устройствам
- `COUNT(*)` - подсчет количества логинов
- `ORDER BY ... DESC` - сортировка по убыванию
- `LIMIT 5` - ограничение вывода топ-5 результатов


## Задание 2: Среднее число логинов в день
Получить среднее число логинов в день за последние 7 дней.

### Решение:
```sql
SELECT 
    COUNT(*) / 7.0 AS average_logins_per_day
FROM players
WHERE 
    login_time >= CURRENT_DATE - INTERVAL '7 days'
    AND login_time < CURRENT_DATE + INTERVAL '1 day';
```

**Пояснение:**
- `CURRENT_DATE - INTERVAL '7 days'` - дата 7 дней назад
- `login_time < ... + INTERVAL '1 day'` - включение текущего дня
- `COUNT(*) / 7.0` - расчет среднего (7.0 для дробного результата)

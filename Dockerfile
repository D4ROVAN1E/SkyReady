# Используем Debian Slim (значительно легче Ubuntu, но полностью совместим)
FROM debian:bookworm-slim

# Отключаем интерактивные вопросы
ENV DEBIAN_FRONTEND=noninteractive

# Установка зависимостей
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    qtbase5-dev \
    qt5-qmake \
    libqt5sql5-psql \
    libpq-dev \
    libgl1-mesa-glx \
    libgl1-mesa-dri \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Настройка рабочей директории
WORKDIR /app

# Копируем файлы проекта
COPY . /app

# Сборка проекта
RUN qmake SkyReady.pro && make

# Команда запуска
CMD ["./SkyReady"]

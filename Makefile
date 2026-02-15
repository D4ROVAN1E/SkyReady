# Имя файла с образом и файла конфигурации
IMAGE_ARCHIVE := skyready_app.tar
COMPOSE_FILE := docker-compose.prod.yml

# Автоматическое определение команды: 'docker compose' (v2) или 'docker-compose' (v1)
DOCKER_COMPOSE := $(shell if docker compose version > /dev/null 2>&1; then echo "docker compose"; else echo "docker-compose"; fi)

# Объявляем цели, которые не являются файлами
.PHONY: all load up down clean logs

# Главная команда: загрузка образа и запуск
all: load up

# Загрузка образа из файла .tar в Docker
load:
	@echo "--> Loading Docker image from $(IMAGE_ARCHIVE)..."
	docker load -i $(IMAGE_ARCHIVE)

# Запуск контейнеров в фоновом режиме (-d)
up:
	@echo "--> Starting SkyReady application..."
	$(DOCKER_COMPOSE) -f $(COMPOSE_FILE) up -d
	@echo "--> Application started! Check X11 window."

# Дополнительно: Остановка приложения
down:
	@echo "--> Stopping application..."
	$(DOCKER_COMPOSE) -f $(COMPOSE_FILE) down

# Дополнительно: Полная очистка (остановка + удаление томов базы данных)
clean:
	@echo "--> Cleaning up containers and volumes..."
	$(DOCKER_COMPOSE) -f $(COMPOSE_FILE) down -v

# Просмотр логов
logs:
	$(DOCKER_COMPOSE) -f $(COMPOSE_FILE) logs -f

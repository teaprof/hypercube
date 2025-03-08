#!/bin/bash
docker compose -f airflow/docker-compose.yaml up --build #--no-cache

#docker compose -f airflow/docker-compose.yaml build --no-cache
#docker compose -f airflow/docker-compose.yaml up airflow-init
#docker compose -f airflow/docker-compose.yaml up

#docker compose build #--no-cache
#docker compose up airflow-init
#docker compose up

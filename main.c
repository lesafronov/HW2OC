#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_ROOMS 30
#define NUM_CLIENTS 50

int rooms[NUM_ROOMS]; //Массив номеров, 0 означает свободный номер
sem_t room_sem; //Семафор для ограничения доступа к массиву номеров
sem_t queue_sem; //Семафор для организации очереди клиентов
int counter_for_srand = 0; //Счетчик для организации рандома

void *client(void *arg) {
    srand(time(NULL) + counter_for_srand++);
    int id = *(int *) arg;
    int num_nights = rand() % 5 + 1; // Количество ночей, которое хочет провести клиент
    printf("Client %d wants to stay for %d nights\n", id, num_nights);

    // Ищем свободный номер или ждем, если нет свободных
    int room_num = -1;
    while (room_num == -1) {
        sem_wait(&room_sem);
        for (int i = 0; i < NUM_ROOMS; i++) {
            if (rooms[i] == 0) {
                rooms[i] = id;
                room_num = i;
                break;
            }
        }
        sem_post(&room_sem);
        if (room_num == -1) {
            printf("Client %d is waiting\n", id);
            sem_wait(&queue_sem);
        }
    }

    // Занимаем номер
    printf("Client %d got room %d\n", id, room_num);

    // Ждем нужное количество ночей
    sleep(num_nights);

    // Освобождаем номер
    sem_wait(&room_sem);
    rooms[room_num] = 0;
    sem_post(&room_sem);
    printf("Client %d left room %d\n", id, room_num);

    // Разрешаем следующему клиенту занять номер
    sem_post(&queue_sem);
    pthread_exit(NULL);
}

int main() {
    pthread_t clients[NUM_CLIENTS];
    int client_ids[NUM_CLIENTS];

    // Инициализируем семафоры
    sem_init(&room_sem, 0, 1);
    sem_init(&queue_sem, 0, 0);

    // Инициализируем массив номеров
    for (int i = 0; i < NUM_ROOMS; i++) {
        rooms[i] = 0;
    }

    // Создаем потоки для клиентов
    for (int i = 0; i < NUM_CLIENTS; i++) {
        client_ids[i] = i + 1;
        pthread_create(&clients[i], NULL, client, &client_ids[i]);
    }

    // Ждем, пока все клиенты не покинут гостиницу
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(clients[i], NULL);
    }

    // Удаляем семафоры
    sem_destroy(&room_sem);
    sem_destroy(&queue_sem);

    return 0;
}
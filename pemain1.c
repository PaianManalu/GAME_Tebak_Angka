#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_NAME_LENGTH 50
#define MAX_KISI_LENGTH 1000

// Struct untuk data bersama antara pemain 1 dan pemain 2
struct SharedData {
    char kata[MAX_NAME_LENGTH];
    char kisi1[MAX_KISI_LENGTH];
    char kisi2[MAX_KISI_LENGTH];
    char kisi3[MAX_KISI_LENGTH];
};

// ANSI escape codes untuk format teks
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_SIZE "\033[8m"
#define ANSI_COLOR_BLUE "\033[34m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_RED "\033[31m"

int main() {
    char namaPemain[MAX_NAME_LENGTH];

    printf(ANSI_BOLD ANSI_SIZE ANSI_COLOR_BLUE "Selamat datang di game tebak kata!\n");
    printf("Mari kita mulai.\n" ANSI_COLOR_RESET);

    // Meminta nama pemain 1
    printf("Masukkan nama Pemain 1: ");
    scanf("%s", namaPemain);

    // Membuat kunci dari path file menggunakan ftok
    key_t key = ftok("pemain1.c", 'R');
    if (key == -1) {
        perror(ANSI_COLOR_YELLOW "ftok" ANSI_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    // Mendapatkan atau membuat shared memory
    int shmid = shmget(key, sizeof(struct SharedData), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror(ANSI_COLOR_YELLOW "shmget" ANSI_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    // Menyambungkan program ke shared memory
    struct SharedData *sharedData = (struct SharedData *)shmat(shmid, (void *)0, 0);
    if (sharedData == (void *)-1) {
        perror(ANSI_COLOR_YELLOW "shmat" ANSI_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    int lanjut;
    int totalSkor = 0;

    srand(time(NULL));

    // Loop untuk setiap ronde (maksimum 10 ronde)
    for (int ronde = 1; ronde <= 10; ronde++) {
        printf("\n%sRonde %d - %s:\n", ANSI_COLOR_YELLOW, ronde, namaPemain);

        printf("%s, sekarang giliran Anda membuat kata!\n", namaPemain);
        printf("Masukkan kata: ");
        scanf("%s", sharedData->kata);

        // Memasukkan kisi-kisi atau petunjuk (lebih dari tiga kata)
        printf("Masukkan kisi-kisi atau petunjuk (lebih dari tiga kata):\n");
        printf("Yang ke-1: ");
        scanf(" %[^\n]s", sharedData->kisi1);
        printf("Yang ke-2: ");
        scanf(" %[^\n]s", sharedData->kisi2);
        printf("Yang ke-3: ");
        scanf(" %[^\n]s", sharedData->kisi3);

        // Membuka file "pemain2.txt" untuk menulis data kata dan kisi-kisi
        FILE *file = fopen("pemain2.txt", "w");
        if (file == NULL) {
            perror("Error membuka file");
            return 1;
        }

        // Menulis data ke file
        fprintf(file, "%s\n", sharedData->kata);
        fprintf(file, "%s\n", sharedData->kisi1);
        fprintf(file, "%s\n", sharedData->kisi2);
        fprintf(file, "%s\n", sharedData->kisi3);

        fclose(file);

        printf("Kata dan kisi-kisi berhasil dikirim ke Pemain 2.\n");

        if (ronde < 10) {
            // Menanyakan apakah pemain ingin melanjutkan ke ronde berikutnya
            printf("Apakah Anda ingin melanjutkan ke ronde berikutnya? (1: Ya, 0: Tidak): ");
            scanf("%d", &lanjut);
            if (!lanjut) {
                break;
            }
        }
    }

    // Melepaskan shared memory
    shmdt(sharedData);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

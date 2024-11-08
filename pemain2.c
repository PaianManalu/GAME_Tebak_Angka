#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

// Batasan panjang nama dan kisi-kisi
#define MAX_NAME_LENGTH 50
#define MAX_KISI_LENGTH 1000

// Struktur SharedData untuk menyimpan kata dan kisi-kisi
struct SharedData {
    char kata[MAX_NAME_LENGTH];
    char kisi1[MAX_KISI_LENGTH];
    char kisi2[MAX_KISI_LENGTH];
    char kisi3[MAX_KISI_LENGTH];
};

// ANSI escape codes untuk format teks
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_COLOR_BLUE "\033[34m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_RED "\033[31m"

int main() {
    char namaPemain[MAX_NAME_LENGTH];
    int lanjut;
    int totalSkor = 0;

    // Pesan selamat datang
    printf(ANSI_BOLD ANSI_COLOR_BLUE "Selamat datang di game tebak kata!\n");
    printf("Mari kita mulai.\n" ANSI_COLOR_RESET);

    // Meminta nama Pemain 2
    printf("Masukkan nama Pemain 2: ");
    scanf("%s", namaPemain);

    // Membuat kunci untuk shared memory
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

    // Inisialisasi seed untuk generator bilangan acak
    srand(time(NULL));

    // Loop permainan untuk 10 ronde
    for (int ronde = 1; ronde <= 10; ronde++) {
        // Menampilkan informasi ronde
        printf("\n%sRonde %d - %s:\n", ANSI_COLOR_YELLOW, ronde, namaPemain);

        // Menampilkan kisi-kisi yang diterima dari Pemain 1
        printf("Kisi 1: %s\n", sharedData->kisi1);
        printf("Kisi 2: %s\n", sharedData->kisi2);
        printf("Kisi 3: %s\n", sharedData->kisi3);

        int chance;
        int skorRonde = 0;  // Skor untuk ronde ini

        // Loop untuk menebak kata dengan maksimal 5 kesempatan
        for (chance = 1; chance <= 5; chance++) {
            printf("Tebak kata (Kesempatan ke-%d): ", chance);
            char jawaban[MAX_NAME_LENGTH];
            scanf("%s", jawaban);

            // Memeriksa apakah jawaban benar
            if (strcmp(sharedData->kata, jawaban) == 0) {
                printf(ANSI_COLOR_YELLOW "Benar!\n" ANSI_COLOR_RESET);
                skorRonde += 10;  // Poin untuk jawaban benar
                break;
            } else {
                printf(ANSI_COLOR_RED "Salah! Coba lagi.\n" ANSI_COLOR_RESET);
                if (chance == 5) {
                    printf("Kesempatan habis. Jawaban yang benar adalah: %s\n", sharedData->kata);
                }
            }
        }

        // Menambahkan skor ronde ke total skor
        totalSkor += skorRonde;

        // Menampilkan skor untuk ronde dan skor total
        printf("Skor untuk Ronde %d: %d\n", ronde, skorRonde);
        printf("Skor Total: %d\n", totalSkor);

        // Menanyakan apakah pemain ingin melanjutkan ke ronde berikutnya
        if (ronde < 10) {
            printf("Apakah Anda ingin melanjutkan ke ronde berikutnya? (1: Ya, 0: Tidak): ");
            scanf("%d", &lanjut);
            if (!lanjut) {
                break;  // Keluar dari loop jika tidak ingin melanjutkan
            }
        }
    }

    // Melepaskan program dari shared memory
    shmdt(sharedData);

    // Mengembalikan nilai 0 sebagai tanda program berakhir dengan sukses
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int main() {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

    printf("[КБ V16] Инициализация SocketCAN на Си интерфейса can0...\n");

    // Открытие стандартного сетевого SocketCAN сокета
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("[-] Ошибка открытия CAN сокета");
        return 1;
    }

    strcpy(ifr.ifr_name, "can0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("[-] Ошибка определения индекса интерфейса can0");
        close(s);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[-] Ошибка привязки сокета к can0");
        close(s);
        return 1;
    }

    printf("[КБ V16] Мониторинг сети «Вал Свободы» на Си запущен. Ожидание фреймов...\n");

    while (1) {
        int nbytes = read(s, &frame, sizeof(struct can_frame));
        if (nbytes < 0) {
            perror("[-] Ошибка чтения фрейма");
            continue;
        }

        // Разбор фрейма основного ядра автоматики ДВС
        if (frame.can_id == 0x100 && frame.can_dlc == 4) {
            uint16_t thermo_raw = (frame.data[0] << 8) | frame.data[1];
            uint16_t co_value = (frame.data[2] << 8) | frame.data[3];
            float actual_temp = (thermo_raw >> 3) * 0.25f; // Точный перевод в градусы Цельсия
            printf("[ТЕЛЕМЕТРИЯ ЯДРА] Т: %.2f°C | Уровень CO (MQ-9): %d\n", actual_temp, co_value);
        }
        // Разбор фрейма солевого пола HighStar
        else if (frame.can_id == 0x101 && frame.can_dlc == 3) {
            uint16_t salt_mv = (frame.data[0] << 8) | frame.data[1];
            uint8_t state_code = frame.data[2];
            const char* state_str = "НЕИЗВЕСТНО";
            if (state_code == 0) state_str = "СТОП";
            else if (state_code == 1) state_str = "ПУСК С КУРАНТОВ";
            else if (state_code == 2) state_str = "МАРШ ГАЗ";
            else if (state_code == 3) state_str = "БЕСШУМНЫЙ ПАР";

            printf("[БАТАРЕЯ HighStar] Заряд: %.2fV | Режим: %s\n", (float)salt_mv / 1000.0f, state_str);
        }
        // Разбор фрейма положения приводов
        else if (frame.can_id == 0x102 && frame.can_dlc == 3) {
            uint16_t nema_pos = (frame.data[0] << 8) | frame.data[1];
            uint8_t azot_state = frame.data[2];
            printf("[ПРИВОДЫ] ШРУС-клапан NEMA 17: %d шаг | Азотный Рубильник: %s\n", 
                   nema_pos, (azot_state == 1) ? "ЗАПЕРТ (СТОП ТОПКИ)" : "ОТКРЫТ (МАРШ)");
        }
    }

    close(s);
    return 0;
}

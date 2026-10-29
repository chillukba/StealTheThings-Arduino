import time
import pygame
import serial

PORT_ARDUINO = 'COM8'
BAUD_RATE = 9600
AUDIO_FILE = 'alarm_sound.mp3'

print('Menghubungkan ke Arduino...')
try:
    ser = serial.Serial(PORT_ARDUINO, BAUD_RATE, timeout=0.1)
    time.sleep(2)
    print('Berhasil terhubung ke Arduino!')
except Exception as e:
    print(f'Gagal terhubung ke Arduino: {e}')
    exit()

pygame.mixer.init()

try:
    shared_sound = pygame.mixer.Sound(AUDIO_FILE)

    touch_channel = pygame.mixer.Channel(0)
    alarm_channel = pygame.mixer.Channel(1)

    print('File audio siap. Menunggu sinyal dari Arduino...')
except Exception as e:
    print(f'Error memuat audio: {e}')
    exit()

while True:
    try:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()

            if line:
                print(f'[Arduino]: {line}')

                if line == 'ARDUINO_READY':
                    pygame.mixer.stop()

                elif line == 'TOUCH_PRESSED':
                    touch_channel.play(shared_sound, loops=0)
                elif line == 'TOUCH_RELEASED':
                    touch_channel.stop()

                elif line == 'PLAY_AUDIO_BUTTON_B_RELEASED':
                    if not alarm_channel.get_busy():
                        alarm_channel.play(shared_sound, loops=-1)

                elif line == 'STOP_ALARM':
                    alarm_channel.stop()

    except KeyboardInterrupt:
        print('\nProgram dihentikan.')
        break
    except serial.SerialException as e:
        print(f'Koneksi serial terputus: {e}')
        break
    except Exception as e:
        print(f'Error: {e}')
        time.sleep(0.5)

    time.sleep(0.005)

pygame.mixer.stop()
ser.close()

import time
import pygame
import serial

PORT_ARDUINO = 'COM8'  # Sesuai port kamu
BAUD_RATE = 9600

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
  # Menggunakan 1 file audio yang sama untuk alarm dan touch
  shared_sound = pygame.mixer.Sound('alarm_sound.mp3')

  # Menggunakan dua channel terpisah agar suara touch dan alarm tidak saling bentrok
  touch_channel = pygame.mixer.Channel(0)
  alarm_channel = pygame.mixer.Channel(1)

  print('File audio siap. Menunggu sinyal dari Arduino...')
except Exception as e:
  print(f'Error memuat audio: {e}')
  exit()

while True:
  try:
    if ser.in_waiting > 0:
      line = ser.readline().decode('utf-8').strip()

      if line:
        print(f'[Arduino]: {line}')

        if line == 'ARDUINO_READY' or line == 'SYSTEM_RE-ARMED':
          pygame.mixer.stop()

        # Logika Touch Sensor (Menyala saat disentuh, mati saat dilepas)
        elif line == 'TOUCH_PRESSED':
          # Play sekali (loops=0), diputar di channel terpisah
          touch_channel.play(shared_sound, loops=0)
        elif line == 'TOUCH_RELEASED':
          touch_channel.stop()  # Langsung cut off suara touch

        # Logika Alarm Button B (Loop terus sampai ditekan/QTE sukses)
        elif line == 'PLAY_AUDIO_BUTTON_B_RELEASED':
          alarm_channel.play(shared_sound, loops=-1)

        elif line == 'STOP_ALARM' or line == 'QTE_SUCCESS':
          alarm_channel.stop()

  except KeyboardInterrupt:
    print('\nProgram dihentikan.')
    break
  except Exception as e:
    print(f'Error: {e}')
    time.sleep(1)

  time.sleep(0.005)
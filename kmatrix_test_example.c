/**
 * Simple Keyboard Matrix Test Application
 * 
 * Este é um exemplo de como usar o driver de teclado matricial criado.
 * Pode ser compilado como uma aplicação NuttX ou incluído em um projeto.
 * 
 * Compilação (como exemplo em apps/testing/):
 *   CONFIG_TESTING_KMATRIX_TEST=y
 *   CONFIG_TESTING_KMATRIX_TEST_STACKSIZE=2048
 * 
 * Execução:
 *   nsh> kmatrix_test
 * 
 * Saída esperada:
 *   Keyboard Matrix Test v1.0
 *   Opening /dev/kbd0...
 *   Waiting for key events...
 *   Press any key...
 *   
 *   Key pressed: 0x31 (1) [PRESS]
 *   Key released: 0x31 (1) [RELEASE]
 *   Key pressed: 0x2a (*) [PRESS]
 *   Key released: 0x2a (*) [RELEASE]
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <nuttx/input/keyboard.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define KBD_DEVPATH "/dev/kbd0"

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Key code to ASCII mapping for standard phone keypad */

static const char *g_key_names[] =
{
  [0x00] = "NULL",
  [0x31] = "1",
  [0x32] = "2",
  [0x33] = "3",
  [0x34] = "4",
  [0x35] = "5",
  [0x36] = "6",
  [0x37] = "7",
  [0x38] = "8",
  [0x39] = "9",
  [0x2a] = "*",
  [0x30] = "0",
  [0x23] = "#",
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * Get printable name for a key code
 */

static const char *get_key_name(uint8_t code)
{
  if (code < sizeof(g_key_names) / sizeof(g_key_names[0]))
    {
      if (g_key_names[code])
        {
          return g_key_names[code];
        }
    }

  return "?";
}

/**
 * Get readable name for keyboard event type
 */

static const char *get_event_type(uint32_t type)
{
  switch (type)
    {
      case KEYBOARD_PRESS:
        return "PRESS";
      case KEYBOARD_RELEASE:
        return "RELEASE";
      default:
        return "UNKNOWN";
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * Main test function
 */

int main(int argc, FAR char *argv[])
{
  int fd;
  struct keyboard_event_s event;
  ssize_t nread;
  int num_events = 0;

  printf("\n");
  printf("========================================\n");
  printf("Keyboard Matrix Test v1.0\n");
  printf("========================================\n\n");

  /* Open keyboard device */

  printf("Opening %s...\n", KBD_DEVPATH);
  fd = open(KBD_DEVPATH, O_RDONLY);

  if (fd < 0)
    {
      printf("ERROR: Failed to open %s\n", KBD_DEVPATH);
      return 1;
    }

  printf("OK! Device opened\n\n");
  printf("Waiting for key events...\n");
  printf("Press keys on the matrix, or Ctrl+C to exit\n\n");

  printf("Event Log:\n");
  printf("----------\n");

  /* Read and display events */

  while (1)
    {
      nread = read(fd, &event, sizeof(event));

      if (nread == sizeof(event))
        {
          num_events++;

          printf("[%3d] Key 0x%02x (%s) [%s]\n",
                 num_events,
                 event.code,
                 get_key_name(event.code),
                 get_event_type(event.type));

          fflush(stdout);
        }
      else if (nread < 0)
        {
          printf("\nERROR: read() failed: %ld\n", (long)nread);
          break;
        }
      else if (nread == 0)
        {
          /* Usually should not happen in blocking mode */

          printf("DEBUG: read() returned 0 bytes (EOF?)\n");
          usleep(100000);  /* 100ms sleep before retry */
        }
    }

  printf("\n");
  printf("========================================\n");
  printf("Test ended - Total events: %d\n", num_events);
  printf("========================================\n\n");

  close(fd);
  return 0;
}

/**
 * Advanced Test: With Poll
 * 
 * Uncomment para usar poll() em vez de read() bloqueante
 * Útil para testar múltiplos devices ou com timeout
 */

#ifdef KMATRIX_TEST_WITH_POLL

#include <poll.h>

int main_with_poll(int argc, FAR char *argv[])
{
  int fd;
  struct pollfd pfd;
  struct keyboard_event_s event;
  ssize_t nread;
  int num_events = 0;
  int poll_count = 0;

  printf("Keyboard Matrix Test with poll() - v1.0\n\n");

  fd = open(KBD_DEVPATH, O_RDONLY);
  if (fd < 0)
    {
      printf("ERROR: Failed to open %s\n", KBD_DEVPATH);
      return 1;
    }

  pfd.fd = fd;
  pfd.events = POLLIN;

  printf("Polling %s with 5 second timeout...\n\n", KBD_DEVPATH);

  while (1)
    {
      int ret = poll(&pfd, 1, 5000);  /* 5 second timeout */

      poll_count++;

      if (ret > 0)
        {
          if (pfd.revents & POLLIN)
            {
              nread = read(fd, &event, sizeof(event));

              if (nread == sizeof(event))
                {
                  num_events++;
                  printf("[%d] Key 0x%02x: %s\n",
                         num_events,
                         event.code,
                         get_event_type(event.type));
                }
            }
        }
      else if (ret == 0)
        {
          printf("Poll timeout (no events for 5s) - %d\n", poll_count);
        }
      else
        {
          printf("Poll error\n");
          break;
        }
    }

  printf("Total events: %d\n", num_events);
  close(fd);
  return 0;
}

#endif  /* KMATRIX_TEST_WITH_POLL */

/**
 * Teste de Performance
 * 
 * Conta eventos por segundo e verifica taxa de polling
 */

#ifdef KMATRIX_TEST_PERFORMANCE

#include <time.h>

int main_performance(int argc, FAR char *argv[])
{
  int fd;
  struct keyboard_event_s event;
  ssize_t nread;
  int count = 0;
  time_t start, end;
  double duration;

  printf("Keyboard Matrix Performance Test\n\n");

  fd = open(KBD_DEVPATH, O_RDONLY);
  if (fd < 0)
    {
      printf("ERROR: Failed to open %s\n", KBD_DEVPATH);
      return 1;
    }

  printf("Reading events for 10 seconds...\n");
  printf("Press keys rapidly...\n\n");

  start = time(NULL);
  end = start + 10;  /* 10 seconds */

  while (time(NULL) < end)
    {
      nread = read(fd, &event, sizeof(event));
      if (nread == sizeof(event))
        {
          count++;
        }
    }

  duration = difftime(end, start);
  printf("Events received: %d\n", count);
  printf("Duration: %.1f seconds\n", duration);
  printf("Event rate: %.1f events/sec\n", (double)count / duration);

  close(fd);
  return 0;
}

#endif  /* KMATRIX_TEST_PERFORMANCE */

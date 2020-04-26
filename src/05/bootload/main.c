#include "defines.h"
#include "serial.h"
#include "xmodem.h"
#include "lib.h"

static int init(void)
{
  /* �ȉ��̓����J�E�X�N���v�g�Œ�`���Ă���V���{�� */
  extern int erodata, data_start, edata, bss_start, ebss;

  /*
   * �f�[�^�̈��BSS�̈������������.���̏����ȍ~�łȂ���,
   * �O���[�o���ϐ�������������Ă��Ȃ��̂Œ���.
   */
  memcpy(&data_start, &erodata, (long)&edata - (long)&data_start);
  memset(&bss_start, 0, (long)&ebss - (long)&bss_start);

  /* �V���A���̏����� */
  serial_init(SERIAL_DEFAULT_DEVICE);

  return 0;
}

/* ��������16�i�_���v�o�� */
static int dump(char *buf, long size)
{
  long i;

  if (size < 0){
    puts("no data.\n");
    return -1;
  }
  for (i = 0; i < size; i++) {
    putxval(buf[i], 2);
    if ((i & 0xf) == 15) {
      puts("\n");
    } else {
      if ((i & 0xf) == 7) puts(" ");
      puts(" ");
    }
  }
  puts("\n");
}

static void wait()
{
  volatile long i;
  for (i = 0; i < 300000; i++)
    ;
}
    
int main(void)
{
  static char buf[16];
  static long size = -1;
  static unsigned char *loadbuf = NULL;
  extern int buffer_start; /* �����J�E�X�N���v�g�Œ�`����Ă���o�b�t�@ */

  init();

  puts("kzload (kozos boot loader) started.\n");

  while (1) {
    puts("kzload> "); /* �v�����v�g�\�� */
    gets(buf); /* �V���A������̃R�}���h��M */

    if (!strcmp(buf, "load")) { /* XMODEM�ł̃t�@�C���̃_�E�����[�h */
      loadbuf = (char *)(&buffer_start);
      size = xmodem_recv(loadbuf);
      wait(); /* �]���A�v�����I�����[���A�v���ɐ��䂪�߂�܂ő҂����킹�� */
      if (size < 0) {
        puts("\nXMODEM receive error!\n");
      } else {
        puts("\nXMODEM receive succeeded.\n");
      }
    } else if (!strcmp(buf, "dump")) { /* ��������16�i�_���v�o�� */
      puts("size: ");
      putxval(size, 0);
      puts("\n");
      dump(loadbuf, size);
    } else if (!strcmp(buf, "run")) { /* ELF�`���t�@�C���̎��s */
      elf_load(loadbuf); /* ��������ɓW�J�i���[�h�j */
    } else {
      puts("unknown.\n");
    }
  }

  return 0;
}

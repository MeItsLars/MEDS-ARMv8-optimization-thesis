#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "bitstream.h"

int bs_init(bitstream_t *bs, uint8_t *buf, size_t buf_len)
{
  bs->data = buf;
  bs->buf_len = buf_len;
  bs->byte_pos = 0;
  bs->bit_pos = 0;

  return 0;
}

int bs_finalize(bitstream_t *bs)
{
  bs->byte_pos += (bs->bit_pos > 0 ? 1 : 0);
  bs->bit_pos = 0;
  return bs->byte_pos - 1;
}

int bs_write(bitstream_t *bs, uint32_t data, uint32_t data_len)
{
  if (bs->byte_pos * 8 + bs->bit_pos + data_len > bs->buf_len * 8)
  {
    fprintf(stderr, "ERROR: bistream - write esceeds buffer!\n");
    return -1;
  }

  if (bs->bit_pos == 0)
    bs->data[bs->byte_pos] = 0;

  if (bs->bit_pos + data_len < 8)
  {
    bs->data[bs->byte_pos] |= data << bs->bit_pos;

    bs->bit_pos += data_len;

    if (bs->bit_pos > 7)
    {
      bs->bit_pos = 0;
      bs->byte_pos += 1;
    }

    return 0;
  }

  if (bs->bit_pos > 0)
  {
    bs->data[bs->byte_pos] |= (data << bs->bit_pos) & 0xFF;

    data >>= 8 - bs->bit_pos;
    data_len -= 8 - bs->bit_pos;

    bs->bit_pos = 0;
    bs->byte_pos += 1;
  }

  while (data_len >= 8)
  {
    bs->data[bs->byte_pos] = data & 0xFF;

    data >>= 8;
    data_len -= 8;

    bs->byte_pos += 1;
  }

  if (data_len > 0)
  {
    bs->data[bs->byte_pos] = data;

    bs->bit_pos = data_len;
  }

  return 0;
}

uint32_t bs_read(bitstream_t *bs, uint32_t data_len)
{
  if (bs->byte_pos * 8 + bs->bit_pos + data_len > bs->buf_len * 8)
  {
    fprintf(stderr, "ERROR: bistream - read esceeds buffer!\n");
    return -1;
  }

  uint32_t data = 0;

  if (bs->bit_pos + data_len < 8)
  {
    data = (bs->data[bs->byte_pos] >> bs->bit_pos) & ((1 << data_len)-1);

    bs->bit_pos += data_len;

    if (bs->bit_pos > 7)
    {
      bs->bit_pos = 0;
      bs->byte_pos += 1;
    }

    return data;
  }

  uint32_t off = 0;

  if (bs->bit_pos > 0)
  {
    data = bs->data[bs->byte_pos] >> bs->bit_pos;

    off = 8-bs->bit_pos;
    data_len -= 8-bs->bit_pos;

    bs->bit_pos = 0;
    bs->byte_pos += 1;
  }

  while (data_len >= 8)
  {
    data |= bs->data[bs->byte_pos] << off;
    off += 8;
    bs->byte_pos += 1;
    data_len -= 8;
    bs->bit_pos = 0;
  }

  if (data_len > 0)
  {
    data |= (bs->data[bs->byte_pos] & ((1 <<  data_len)-1)) << off;

    bs->bit_pos = data_len;
  }

  return data;
}

#ifdef TEST
void main()
{
  uint32_t data[10] = {29985, 56003, 204, 4, 0, 970, 55415, 12867, 3, 427};

  uint32_t widths[10] = {16, 18, 9, 3, 2, 12, 17, 15, 3, 11};

  uint8_t bs_buf[256];
  bitstream_t bs;

  bs_init(&bs, bs_buf, sizeof(bs_buf));

  for (int i = 0; i < 10; i++)
    bs_write(&bs, data[i], widths[i]);

  bs_finalize(&bs);

  bs_init(&bs, bs_buf, sizeof(bs_buf));

  for (int i = 0; i < 10; i++)
  {
    uint32_t val;

    val = bs_read(&bs, widths[i]);

    printf("%u %u\n", data[i], val);
  }
}
#endif


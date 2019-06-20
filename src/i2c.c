#include "mruby.h"
#include "mruby/variable.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/array.h"
#include "mruby/data.h"
#include <string.h>

#ifdef SOFTDEVICE_PRESENT
  // #include "nrf_util_platform.h"
  // #include "nrf_gpio.h"
  #include "nrf_drv_twi.h"
  #include "nrf_delay.h"
  #include "nrf_log.h"

  // get TWI instance (external function)
  extern nrf_drv_twi_t const * getTwiInstance(void);
#endif

// I2C.new(addr, wait) => I2C
static mrb_value
mrb_i2c_init(mrb_state *mrb, mrb_value self)
{
  mrb_int addr;
  mrb_int wait = 0;

  mrb_get_args(mrb, "i|i", &addr, &wait);

  /* initialize I2C instance */
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@addr"), mrb_fixnum_value(addr));
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@wait"), mrb_fixnum_value(wait));

  return self;
}

// i2c._read(len, type)
static mrb_value
mrb_i2c_read(mrb_state *mrb, mrb_value self)
{
  mrb_int len;
  mrb_sym type = mrb_intern_lit(mrb, "as_array");
  uint8_t *buf;
  mrb_value v, vaddr;

  mrb_get_args(mrb, "i|n", &len, &type);
  vaddr = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@addr"));

  buf = mrb_malloc(mrb, len);
  memset(buf, 0, len);

  /* read I2C data */
#ifdef SOFTDEVICE_PRESENT
  if (nrf_drv_twi_rx(getTwiInstance(), (uint8_t)mrb_fixnum(vaddr), buf, len) != NRF_SUCCESS) {
    mrb_free(mrb, buf);
    // TODO: raise exception
    return mrb_false_value();
  }
#endif

  if (type == mrb_intern_lit(mrb, "as_string")) {
    /* as_string */
    v = mrb_str_buf_new(mrb, len);
    memcpy(RSTRING_PTR(v), buf, len);
    mrb_str_resize(mrb, v, len);
  }
  else {
    int i;
    /* as_array or other */
    v = mrb_ary_new_capa(mrb, len);
    for (i=0; i<len; i++) {
      mrb_ary_push(mrb, v, mrb_fixnum_value(buf[i]));
    }
  }
  /* free read buffer */
  mrb_free(mrb, buf);

  return v;
}

// i2c._write(cmd) => true
static mrb_value
mrb_i2c_write(mrb_state *mrb, mrb_value self)
{
  mrb_value cmds;
  uint8_t *buf;
  size_t len, i;
  mrb_value vaddr;

  mrb_get_args(mrb, "o", &cmds);
  vaddr = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@addr"));

  if (mrb_string_p(cmds)) {
    len = RSTRING_LEN(cmds);
    buf = mrb_malloc(mrb, len);
    memcpy(buf, RSTRING_PTR(cmds), len);
  }
  else if (mrb_array_p(cmds)) {
    len = RARRAY_LEN(cmds);
    buf = mrb_malloc(mrb, len);
    for (i=0; i<len; i++) {
      buf[i] = (uint8_t)mrb_fixnum(mrb_ary_ref(mrb, cmds, i));
    }
  }
  else if (mrb_fixnum_p(cmds)) {
    len = 1;
    buf = mrb_malloc(mrb, len);
    buf[0] = mrb_fixnum(cmds);
  }
  else {
    mrb_value v = mrb_obj_as_string(mrb, cmds);
    len = RSTRING_LEN(v);
    buf = mrb_malloc(mrb, len);
    memcpy(buf, RSTRING_PTR(v), len);
  }

#ifdef SOFTDEVICE_PRESENT
  /* write data to I2C */
  if (nrf_drv_twi_tx(getTwiInstance(), (uint8_t)mrb_fixnum(vaddr), buf, len, false) != NRF_SUCCESS) {
    // TODO: raise exception
    return mrb_false_value();
  }
#endif

  /* free command data */
  mrb_free(mrb, buf);

  return mrb_true_value();
}

static mrb_value
mrb_i2c_start(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

static mrb_value
mrb_i2c_end(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

void
mrb_mruby_plato_i2c_nrf52_gem_init(mrb_state *mrb)
{
  struct RClass *mod  = mrb_define_module(mrb, "PlatoNRF52");
  struct RClass *i2c  = mrb_define_class_under(mrb, mod, "I2C", mrb->object_class);
  MRB_SET_INSTANCE_TT(i2c, MRB_TT_DATA);

  mrb_define_method(mrb, i2c, "initialize", mrb_i2c_init,     MRB_ARGS_ARG(1, 1));
  mrb_define_method(mrb, i2c, "_read",      mrb_i2c_read,     MRB_ARGS_ARG(1, 1));
  mrb_define_method(mrb, i2c, "write",      mrb_i2c_write,    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, i2c, "_start",     mrb_i2c_start,    MRB_ARGS_NONE());
  mrb_define_method(mrb, i2c, "_end",       mrb_i2c_end,      MRB_ARGS_NONE());
}

void
mrb_mruby_plato_i2c_nrf52_gem_final(mrb_state *mrb)
{
}

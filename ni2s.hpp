#include "driver/i2s_std.h"
#include "driver/i2s_tdm.h"
#include "esp_err.h"
#include "nlogger.hpp"

#include <type_traits>

namespace nix
{

struct I2sBusConfig : public i2s_chan_config_t
{

    I2sBusConfig(
        i2s_port_t id, 
        i2s_role_t role, 
        uint32_t dma_desc_num, 
        uint32_t dma_frame_num, 
        bool auto_clear_after_cb,
        bool auto_clear_before_cb,
        int intr_priority
      ) : i2s_chan_config_t{}  // Zero-initialize base struct using aggregate initialization
    {
        // Set the actual values
        this->id = id;
        this->role = role;
        this->dma_desc_num = dma_desc_num;
        this->dma_frame_num = dma_frame_num;
        this->auto_clear_after_cb = auto_clear_after_cb;
        this->auto_clear_before_cb = auto_clear_before_cb;
        this->intr_priority = intr_priority;
    }
};

struct I2sChanStdConfig : public i2s_std_config_t
{
    I2sChanStdConfig( 
        uint32_t sample_rate_hz,
        i2s_clock_src_t clk_src,
        uint32_t ext_clk_freq_hz,
        i2s_mclk_multiple_t mclk_multiple,
        i2s_data_bit_width_t data_bit_width,
        i2s_slot_bit_width_t slot_bit_width,
        i2s_slot_mode_t slot_mode,
        i2s_std_slot_mask_t slot_mask,
        uint32_t ws_width,
        bool ws_pol,
        bool bit_shift,
        bool left_align,
        bool big_endian,
        bool bit_order_lsb,
        gpio_num_t mclk,
        gpio_num_t bclk,
        gpio_num_t ws,
        gpio_num_t dout,
        gpio_num_t din,
        bool mclk_inv = false,
        bool bclk_inv = false,
        bool ws_inv = false) : i2s_std_config_t{}  // Zero-initialize base struct using aggregate initialization
    {
        clk_cfg.sample_rate_hz  = sample_rate_hz;
        clk_cfg.clk_src         = clk_src;
        clk_cfg.ext_clk_freq_hz = ext_clk_freq_hz;
        clk_cfg.mclk_multiple   = mclk_multiple;

        slot_cfg.data_bit_width = data_bit_width;
        slot_cfg.slot_bit_width = slot_bit_width;
        slot_cfg.slot_mode      = slot_mode;
        slot_cfg.slot_mask      = slot_mask;
        slot_cfg.ws_width       = ws_width;
        slot_cfg.ws_pol         = ws_pol;
        slot_cfg.bit_shift      = bit_shift;
        slot_cfg.left_align     = left_align;
        slot_cfg.big_endian     = big_endian;
        slot_cfg.bit_order_lsb  = bit_order_lsb;

        gpio_cfg.mclk                  = mclk;
        gpio_cfg.bclk                  = bclk;
        gpio_cfg.ws                    = ws;
        gpio_cfg.dout                  = dout;
        gpio_cfg.din                   = din;
        gpio_cfg.invert_flags.mclk_inv = mclk_inv;
        gpio_cfg.invert_flags.bclk_inv = bclk_inv;
        gpio_cfg.invert_flags.ws_inv   = ws_inv;
    }
};

struct I2SChanTdmConfig : public i2s_tdm_config_t
{
    I2SChanTdmConfig(
        uint32_t sample_rate_hz,
        i2s_clock_src_t clk_src,
        uint32_t ext_clk_freq_hz,
        i2s_mclk_multiple_t mclk_multiple,
        uint32_t bclk_div,
        i2s_data_bit_width_t data_bit_width,
        i2s_slot_bit_width_t slot_bit_width,
        i2s_slot_mode_t slot_mode,
        i2s_tdm_slot_mask_t slot_mask,
        uint32_t ws_width,
        bool ws_pol,
        bool bit_shift,
        bool left_align,
        bool big_endian,
        bool bit_order_lsb,
        bool skip_mask,
        uint32_t total_slot,
        gpio_num_t mclk,
        gpio_num_t bclk,
        gpio_num_t ws,
        gpio_num_t dout,
        gpio_num_t din,
        bool mclk_inv = false,
        bool bclk_inv = false,
        bool ws_inv = false): i2s_tdm_config_t{}  // Zero-initialize base struct using aggregate initialization
    {
        clk_cfg.sample_rate_hz = sample_rate_hz;
        clk_cfg.clk_src = clk_src;
        clk_cfg.ext_clk_freq_hz = ext_clk_freq_hz;
        clk_cfg.mclk_multiple = mclk_multiple;
        clk_cfg.bclk_div = bclk_div;

        slot_cfg.data_bit_width = data_bit_width;
        slot_cfg.slot_bit_width = slot_bit_width;
        slot_cfg.slot_mode = slot_mode;
        slot_cfg.slot_mask = slot_mask;
        slot_cfg.ws_width = ws_width;
        slot_cfg.ws_pol = ws_pol;
        slot_cfg.bit_shift = bit_shift;
        slot_cfg.left_align = left_align;
        slot_cfg.big_endian = big_endian;
        slot_cfg.bit_order_lsb = bit_order_lsb;
        slot_cfg.skip_mask = skip_mask;
        slot_cfg.total_slot = total_slot;

        gpio_cfg.mclk = mclk;
        gpio_cfg.bclk = bclk;
        gpio_cfg.ws = ws;
        gpio_cfg.dout = dout;
        gpio_cfg.din = din;
        gpio_cfg.invert_flags.mclk_inv = mclk_inv;
        gpio_cfg.invert_flags.bclk_inv = bclk_inv;
        gpio_cfg.invert_flags.ws_inv = ws_inv;
    }
};

class I2sBus
{
    Logger& m_logger;
    i2s_chan_handle_t m_tx_chan_handle = NULL;
    i2s_chan_handle_t m_rx_chan_handle = NULL;

public:
    I2sBus(Logger& logger) : m_logger(logger) {}
    ~I2sBus();
    esp_err_t Init(I2sBusConfig& bus_config);
    esp_err_t ConfigureTxChannel(I2sChanStdConfig& chan_config);
    esp_err_t ConfigureTxChannel(I2SChanTdmConfig& chan_config);
    esp_err_t ConfigureRxChannel(I2SChanTdmConfig& chan_config);
    esp_err_t ConfigureRxChannel(I2sChanStdConfig& chan_config);
};

class I2sDeviceOutput
{
  Logger& m_logger;
  I2sBus& m_i2s_bus;
  I2sChanStdConfig m_chan_config;
public:
  I2sDeviceOutput(Logger& logger, I2sBus& i2s_bus, const I2sChanStdConfig& chan_config)
    : m_logger(logger), m_i2s_bus(i2s_bus), m_chan_config(chan_config)
  {
  }
};


}; // namespace nix
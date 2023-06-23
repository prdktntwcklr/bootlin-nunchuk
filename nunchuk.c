#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>

/* input poll interval in milliseconds */
#define POLL_INTERVAL (50)

struct nunchuk_dev {
	struct i2c_client *i2c_client;
};

int nunchuk_read_registers(struct i2c_client *client, unsigned char *rx_buffer)
{
	int ret;
	unsigned char tx_buffer[1];

	/* 10ms delay */
	usleep_range(10000, 20000);

	/* write 0x00 to bus to read registers */
	tx_buffer[0] = 0x00;
	ret = i2c_master_send(client, tx_buffer, 1);
	if (ret != 1) {
		dev_err(&client->dev, "i2c send failed (%d)\n", ret);
		return ret < 0 ? ret : -EIO;
	}

	/* 10ms delay */
	usleep_range(10000, 20000);

	/* read nunchuk registers */
	ret = i2c_master_recv(client, rx_buffer, 6);
	if (ret != 6) {
		dev_err(&client->dev, "i2c recv failed (%d)\n", ret);
		return ret < 0 ? ret : -EIO;
	}

	return 0;
}

void nunchuk_poll(struct input_dev *input)
{
	int ret, zpressed, cpressed, bx, by;
	unsigned char rx_buffer[6];

	struct nunchuk_dev *nunchuk = input_get_drvdata(input);
	struct i2c_client *client = nunchuk->i2c_client;

	ret = nunchuk_read_registers(client, rx_buffer);
	if (ret < 0)
		return;

	zpressed = (rx_buffer[5] & BIT(0)) ? 0 : 1;
	cpressed = (rx_buffer[5] & BIT(1)) ? 0 : 1;
	bx = rx_buffer[0];
	by = rx_buffer[1];

	input_report_key(input, BTN_Z, zpressed);
	input_report_key(input, BTN_C, cpressed);

	input_report_abs(input, ABS_X, bx);
	input_report_abs(input, ABS_Y, by);
	
	input_sync(input);
}

int nunchuk_probe(struct i2c_client *client)
{
	int ret;
	unsigned char tx_buffer[2];

	struct input_dev *input;
	struct nunchuk_dev *nunchuk;

	/* initialize device */
	tx_buffer[0] = 0xf0;
	tx_buffer[1] = 0x55;

	ret = i2c_master_send(client, tx_buffer, 2);
	if (ret != 2) {
		dev_err(&client->dev, "i2c send failed (%d)\n", ret);
		return ret < 0 ? ret : -EIO;
	}

	udelay(1000);

	tx_buffer[0] = 0xfb;
	tx_buffer[1] = 0x00;

	ret = i2c_master_send(client, tx_buffer, 2);
	if (ret != 2) {
		dev_err(&client->dev, "i2c send failed (%d)\n", ret);
		return ret < 0 ? ret : -EIO;
	}

	input = devm_input_allocate_device(&client->dev);

	nunchuk = devm_kzalloc(&client->dev, sizeof(*nunchuk), GFP_KERNEL);
	if (!nunchuk)
        return -ENOMEM;

	nunchuk->i2c_client = client;
	input_set_drvdata(input, nunchuk);

	input->name = "Wii Nunchuk";
	input->id.bustype = BUS_I2C;

	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);
	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	input_set_abs_params(input, ABS_X, 30, 220, 4, 8);
	input_set_abs_params(input, ABS_Y, 40, 200, 4, 8);

	/* classic buttons */
	set_bit(BTN_TL, input->keybit);
	set_bit(BTN_SELECT, input->keybit);
	set_bit(BTN_MODE, input->keybit);
	set_bit(BTN_START, input->keybit);
	set_bit(BTN_TR, input->keybit);
	set_bit(BTN_TL2, input->keybit);
	set_bit(BTN_B, input->keybit);
	set_bit(BTN_Y, input->keybit);
	set_bit(BTN_A, input->keybit);
	set_bit(BTN_X, input->keybit);
	set_bit(BTN_TR2, input->keybit);

	ret = input_setup_polling(input, nunchuk_poll);
	if (ret) {
		dev_err(&client->dev, "input setup polling failed (%d)\n", ret);
		return ret;
	}

	input_set_poll_interval(input, POLL_INTERVAL);
	input_set_min_poll_interval(input, POLL_INTERVAL);
	input_set_max_poll_interval(input, POLL_INTERVAL);

	/* register the input device when everything is ready */
	ret = input_register_device(input);
	if (ret < 0) {
		dev_err(&client->dev, "input register device failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

int nunchuk_remove(struct i2c_client *client)
{
	/* nothing to do here */
	return 0;
}

static const struct of_device_id nunchuk_of_match[] = {
	{ .compatible = "nintendo,nunchuk" },
	{ },
};

MODULE_DEVICE_TABLE(of, nunchuk_of_match);

static struct i2c_driver nunchuk_driver = {
	.driver = {
		.name = "nunchuk",
		.of_match_table = nunchuk_of_match,
	},
	.probe_new = nunchuk_probe,
	.remove = nunchuk_remove,
};

module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL");

I2C on Kernel
--------------------------------

## I2C Slave device on DTS

Two way to define a i2c slave describe on DTS.

#### one

```
&i2c0 {
	status = "okay";
	i2cdemo@50 {
		compatible = "i2cdemo,eeprom";
		reg = <0x50>;           
	};
};
```

#### two

```
/{

 ...

	i2c@ff020000 {
		compatible = "cdns,i2c-r1p14", "cdns,i2c-r1p10";
		status = "okay";
		...

		i2cdemo@50 {
			compatible = "i2cdemo,eeprom";
			reg = <0x50>;        
		};
	};
```

## Usage of I2C dirver

On Ubuntu, U can compile directly and running module as follow:

```
make

sudo insmod i2c.ko
```

If U want remove module, as follow:

```
sudo rmmod i2c
```

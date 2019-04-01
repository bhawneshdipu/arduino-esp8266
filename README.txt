CREATE TABLE `sensor` (
	id INT(8) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	status VARCHAR(255) NOT NULL,
	date TIMESTAMP DEFAULT NOW()
)ENGINE = InnoDB CHARSET=utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE TABLE `alarm` (
	id INT(8) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	status VARCHAR(255) NOT NULL,
	date TIMESTAMP DEFAULT NOW()
)ENGINE = InnoDB CHARSET=utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE TABLE `knob` (
	id INT(8) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	status VARCHAR(255) NOT NULL,
	date TIMESTAMP DEFAULT NOW()
)ENGINE = InnoDB CHARSET=utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE TABLE `action` (
	id INT(8) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	action_on VARCHAR(255) NOT NULL,
	action VARCHAR(255) NOT NULL,
	status VARCHAR(255) NOT NULL DEFAULT 'NOT TAKEN',
    date TIMESTAMP DEFAULT NOW()
)ENGINE = InnoDB CHARSET=utf8mb4 COLLATE utf8mb4_unicode_ci;




GET SENSOR STATUS :: http://api.virtualworld.today/sensor_read.php

SEND SENSOR DATA :: http://api.virtualworld.today/sensor_write.php?status=on

GET ALARM STATUS :: http://api.virtualworld.today/alarm_read.php

SEND ALARM DATA :: http://api.virtualworld.today/alarm_write.php?status=on

GET KNOB STATUS :: http://api.virtualworld.today/knob_read.php

SEND KNOB DATA :: http://api.virtualworld.today/knob_write.php?status=on


SEND action data:: http://api.virtualworld.today/action.php?on=knob&action=off
READ action data:: http://api.virtualworld.today/action_read.php?on=knob


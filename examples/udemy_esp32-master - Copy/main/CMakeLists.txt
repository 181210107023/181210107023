# Edit following two lines to set component requirements (see docs)
idf_component_register(SRCS main.c  wifi_app.c http_server.c app_nvs.c wifi_reset_button.c iothub_client_sample_mqtt.c ultrasonic.c
						INCLUDE_DIRS "."
						EMBED_FILES webpage/app.css webpage/app.js webpage/favicon.ico webpage/config.html webpage/index.html webpage/index1.html webpage/jquery-3.3.1.min.js)


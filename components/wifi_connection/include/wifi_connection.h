#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H


/* 
 * @breif connect to the internet and print out the ssid and password
 *
 * initializes the esp-network interface and connects to the wifi 
 * network as defined in sdkconfig through menuconfig. MaxRetry could
 * also be set through there. Blocking in nature :)
 *
 * @return void
 */
void wifi_init_sta(void);

#endif // !WIFI_connection_h

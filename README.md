:zap: esp32 websockets client :zap:
### *part of a project for energy monitoring and device control*

An esp32 websockets client using pusher channels service to make a realtime enery monitoring and remote device control system.
The project sends three phase main voltage and current readings realtime to upto **200 users** at the same time and syncs device contorl *(turning on or off)*.
The web, andriod and ios frontend could be found in my profile.
It also stores the data in a vercel database.
The main features are:
- Three phase AC mains readings
- Realtime data changes
- AC mains current readings
- Control devices (toggle on/off)
- 200 simutaneous users
- Web, Andriod & ios frontends
- Data logging (upto one month)
- Calculates the total electricity bill
---
*written in pure c using espressive iot development framework (esp-idf)*

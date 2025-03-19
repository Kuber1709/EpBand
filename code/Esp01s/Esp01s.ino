#define WIFI_SSID ""
#define WIFI_PASS ""
#define BOT_TOKEN ""
#define CHAT_ID ""


#include <SoftwareSerial.h>
SoftwareSerial mySerial(3, 1);


#include <FastBot2.h>
FastBot2 bot;


struct Settings {
  bool sound_flag = 1;
  bool light_flag = 1;
  char clicks[5] = {'.', '.', '.', '.', '.'};
};
Settings settings;


struct Information {
  String history[20] = {"19.03.2025-21:40", "15.03.2025-12:42", "16.03.2025-19:20"};
  byte pulse = 70;
};
Information information;


bool history_request = 0;
bool pulse_request = 0;
bool clicks_enter = 0;
byte depth = 0;
char clicks_var[5] = {'.', '.', '.', '.', '.'};


void handleMessage(fb::Update& u) {
  if (clicks_enter) {
    bot.deleteMessage(u.message().chat().id(), bot.lastBotMessage());
    clicks_enter = 0;
  }
  String text = u.message().text().decodeUnicode();

  if (text == "/start" || text == "/menu" || text == "Назад 🔙") {
    fb::Message msg("", u.message().chat().id());
    if (text == "/start") msg.text = "Бот успешно активирован ✅";
    else if (text == "/menu") msg.text = "Главное меню 🕹";
    else if (text == "Назад 🔙") msg.text = "Назад 🔙";

    fb::Menu menu;
    menu.text = "История 🕑 ; Пульс ❤ \n Настройки ⚙️";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 0;
  }
  else if (text == "История 🕑" || text == "Пульс ❤") {
    mySerial.write('i');
    if (text == "История 🕑") history_request = 1;
    else if (text == "Пульс ❤") pulse_request = 1;
    fb::Message msg("Запрос отправлен ✅\nОжидание ответа...", u.message().chat().id());
  
    fb::Menu menu;
    menu.text = "История 🕑 ; Пульс ❤ \n Настройки ⚙️";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 0;
  }
  else if (text == "Настройки ⚙️" || text == "Отмена 🚫") {
    fb::Message msg("", u.message().chat().id());
    if (text == "Настройки ⚙️") msg.text = "Настройки ⚙️";
    else if (text == "Отмена 🚫") msg.text = "Отменено 🚫";

    fb::Menu menu;
    menu.text = "Звук 🔈 ; Свет 💡 \n Комбинация нажатий 🔘 \n Назад 🔙";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 1;
  }
  else if (text == "Звук 🔈") {
    fb::Message msg("Звук 🔈\nВо время приступа будет активирован звуковой сигнал", u.message().chat().id());

    fb::Menu menu;
    if (settings.sound_flag) menu.text = "Выключить ❌ \n Отмена 🚫";
    else menu.text = "Включить ✅ \n Отмена 🚫";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 2;
  }
  else if (text == "Свет 💡") {
    fb::Message msg("Свет 💡\nВо время приступа будет активирован световой сигнал", u.message().chat().id());

    fb::Menu menu;
    if (settings.light_flag) menu.text = "Выключить ❌ \n Отмена 🚫";
    else menu.text = "Включить ✅ \n Отмена 🚫";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 3;
  }
  else if (text == "Комбинация нажатий 🔘") {
    fb::Message msg("Комбинация нажатий 🔘\nНастройте комбинацию нажатий кнопки для деактивации сигнала о приступе: _ - зажатие кнопки, . - нажатие кнопки", u.message().chat().id());
    fb::Message imsg("После настройки нажмите Готово ✅", u.message().chat().id());

    fb::Menu menu;
    menu.text = "\n Отмена 🚫";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);    

    fb::InlineMenu imenu;
    for (byte i = 0; i < 5; i++) {
      imenu.addButton(String(settings.clicks[i]), String(i));
      clicks_var[i] = settings.clicks[i];
    }
    imenu.newRow();
    imenu.addButton("Готово ✅", "ready");
    imsg.setInlineMenu(imenu);
    bot.sendMessage(msg);
    bot.sendMessage(imsg);
    depth = 4;
    clicks_enter = 1;
  }
  else if (text == "Включить ✅") {
    if (depth == 2) settings.sound_flag = 1;
    else if (depth == 3) settings.light_flag = 1;
    mySerial.write('s');
    mySerial.write((byte*)&settings, sizeof(settings));
    fb::Message msg("Успешно включено ✅", u.message().chat().id());

    fb::Menu menu;
    menu.text = "Звук 🔈 ; Свет 💡 \n Комбинация нажатий 🔘 \n Назад 🔙";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 1;
  }
  else if (text == "Выключить ❌") {
    if (depth == 2) settings.sound_flag = 0;
    else if (depth == 3) settings.light_flag = 0;
    mySerial.write('s');
    mySerial.write((byte*)&settings, sizeof(settings));
    fb::Message msg("Успешно выключено ❌", u.message().chat().id());

    fb::Menu menu;
    menu.text = "Звук 🔈 ; Свет 💡 \n Комбинация нажатий 🔘 \n Назад 🔙";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 1;
  }
}


void handleQuery(fb::Update& u) {
  if (!clicks_enter) return;
  fb::QueryRead q = u.query();
  if (q.data() == "ready") {
    bot.deleteMessage(q.message().chat().id(), bot.lastBotMessage());
    clicks_enter = 0;

    for (byte i = 0; i < 5; i++) settings.clicks[i] = clicks_var[i];
    mySerial.write('s');
    mySerial.write((byte*)&settings, sizeof(settings));

    fb::Message msg("Сохранено ✅", q.message().chat().id());

    fb::Menu menu;
    menu.text = "Звук 🔈 ; Свет 💡 \n Комбинация нажатий 🔘 \n Назад 🔙";
    menu.resize = 1;
    menu.placeholder = "Выберите пункт:";
    msg.setMenu(menu);

    bot.sendMessage(msg);
    depth = 1;
  }
  else {
    byte i = q.data().toInt();

    if (clicks_var[i] == '.') clicks_var[i] = '_';
    else if (clicks_var[i] == '_') clicks_var[i] = '.';

    fb::MenuEdit m;
    m.chatID = q.message().chat().id();
    m.messageID = q.message().id();

    fb::InlineMenu menu;
    for (byte i = 0; i < 5; i++) menu.addButton(String(clicks_var[i]), String(i));
    menu.newRow();
    menu.addButton("Готово ✅", "ready");

    m.setInlineMenu(menu);
    bot.editMenu(m);
  }
}

void updateh(fb::Update& u) {
  if (u.isMessage()) handleMessage(u);
  if (u.isQuery()) handleQuery(u);
}


void setup() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  mySerial.begin(115200);
  char c = mySerial.read();
  while (c != 's') {
    c = mySerial.read();
  }
  mySerial.readBytes((byte*)&settings, sizeof(settings));

  bot.attachUpdate(updateh);
  bot.setToken(F(BOT_TOKEN));
  bot.setPollMode(fb::Poll::Long, 20000);
  bot.sendMessage(fb::Message("EpBand подлкючён ✅", CHAT_ID));
}


void loop() {
  bot.tick();
  read_arduino();
}


void read_arduino() {
  if (mySerial.available() > 0) {
    delay(100);
    char c = mySerial.read();
    if (c == 'i') {
      mySerial.readBytes((byte*)&information, sizeof(information));
      if (pulse_request) {
        String text = "Получен ответ ✅\nПульс: " + String(information.pulse) + " уд/мин";
        bot.sendMessage(fb::Message(text, CHAT_ID));
      }
      if (history_request) {
        String text = "Получен ответ ✅\nИстория приступов 🕑\n";
        for (byte i = 0; i < 20; i++) text += information.history[i] + "\n";
        bot.sendMessage(fb::Message(text, CHAT_ID));
      }
    }
    else if (c == 'w') {
      bot.sendMessage(fb::Message("❗❗❗ВНИМАНИЕ❗❗❗\nУ пациента зафиксирован приступ!", CHAT_ID));
    }
  }
}

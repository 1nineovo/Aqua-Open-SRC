#pragma once
#include "../../ModuleBase/Module.h"

struct NotificationBox {
    std::string message;
    float duration;
    float maxDuration;
    float PosX;
    float PosY;
    int count;  // 新增：消息计数

    NotificationBox(std::string msg, float dur)
        : message(msg),
          duration(dur),
          maxDuration(dur),
          PosX(0.f),
          PosY(0.f),
          count(1) {}  // 默认计数为1
};

class Notifications : public Module {
   private:
    static inline std::vector<std::shared_ptr<NotificationBox>> notifList;

   public:
    Notifications();
    ~Notifications();
    static void addNotifBox(std::string message, float duration);
    static void Render(ImDrawList* drawlist);

    virtual bool isVisible() override;
};

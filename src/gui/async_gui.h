#pragma once

namespace gui {

class AsyncGui {
   public:
    AsyncGui();
    ~AsyncGui();
    bool deleted;
    virtual void update() = 0;
};

}  // namespace gui
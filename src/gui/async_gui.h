#pragma once

namespace gui {

class AsyncGui {
   public:
    bool deleted;
    virtual void update() = 0;
};

}  // namespace gui
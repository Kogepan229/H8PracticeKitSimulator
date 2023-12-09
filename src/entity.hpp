#pragma once

namespace h8kps {

class EntityBase {
   public:
    EntityBase();
    virtual ~EntityBase();
    bool deleted;
    virtual void update() = 0;
};

}  // namespace h8kps
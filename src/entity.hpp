#pragma once

namespace h8pks {

class EntityBase {
   public:
    EntityBase();
    virtual ~EntityBase();
    bool deleted;
    virtual void update() = 0;
};

}  // namespace h8pks
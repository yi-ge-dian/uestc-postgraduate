#include "single_class_single_object_to_file.h"

int main(){
    Base base(100);
    base.Serialize("data");

    Base newbase = Base::Deserialize("data");
    newbase.print();
    return 0;
}
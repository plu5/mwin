#pragma once

class ConsoleOutputSupport {
 public:
    ConsoleOutputSupport();
    ~ConsoleOutputSupport();
    bool attached = false;
    bool attach();
    bool detach();
};

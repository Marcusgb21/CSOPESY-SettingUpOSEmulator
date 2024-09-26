#pragma once
#include "AConsole.h"

class BaseScreen : public AConsole
{
public:
    BaseScreen(String name);

    void onEnabled() override;
    void offEnabled() override;
    void display() override;
    void process() override;
    void hasExited() override;

private:
    void printProcessData() const;

    bool enabled = false;
    bool exited = false;
    String creationTime;

    void initializeCreationTime();
};
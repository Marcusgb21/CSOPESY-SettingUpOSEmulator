#pragma once
#include "AConsole.h"

class BaseScreen : public AConsole
{
public:
    BaseScreen(String name);

    void onEnabled() override;
    void display() override;
    void process() override;

private:
    void printProcessData() const;

    bool enabled = false;
    String creationTime;

    void initializeCreationTime();
};
struct NotePressInfo
{
    bool mPressed = false;
    unsigned long mUpdatedTime = 0;

    void ChangeState(bool inputPin, unsigned long time);
};


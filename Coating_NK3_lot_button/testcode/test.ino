int limitSwitch = 0;
int state = LOW;
void setup()
{

    Serial.begin(9600);
    pinMode(limitSwitch, INPUT);
}

void loop()
{

    int val = digitalRead(limitSwitch);

    if (val != state)
    {
        state = val;
        Serial.print("Sensor value = ");
        if (state)
            Serial.println(state);
        else
            Serial.println(state);
    }
}
// For now, just the analog read example
void setup() {
	Serial.begin(9600);
}

void loop() {
	const int v = analogRead(A0);
	Serial.println(v);
	delay(1000);
}
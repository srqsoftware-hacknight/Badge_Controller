

// 'call' button for doorbell, use internal pull-up resistor (active low)

// NOTE: gpio2(D4), gpio0(D3) have external 10k pull-up resistors (used in programming)
// We can pull down with 'call' button and check for LOW condition.
// This can be useful to implement a 'Guest Call' button on the reader box.

#define BTN_PIN    D3  // button pin here, tie putton to this pin and GND. Add 100nF across button to help debounce.

#define BTN_PRESS_COUNT    5 // loop count for debounce. Must hold button for about 1/3 second to ring.

#define BTN_LOCKOUT_COUNT  800 // count required until btn is serviced again (lockout after call)
// lockout will help prevent repeating alarms if button held down or pressed repeatedly.
// count  800:  approx 20 secs


int btnCount; // debounce counter, while btn pressed (LOW)
int btnLockout; // lockout counter, after successful call.

void SetupButton() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  btnCount = btnLockout = 0;
}

// #############################################################################
// try read 'call' button, if presed then send to CALL_URL.
//
bool TryButton() {

  // check if lockout from last call.
  if (btnLockout > 0) {
    btnLockout--;
    return false;
  }

  // active low button, LOW when pressed.
  if ( digitalRead(BTN_PIN) == HIGH) {
    btnCount = 0;
    return false;
  }

  if ( digitalRead(BTN_PIN) == LOW) {
    if (++btnCount < BTN_PRESS_COUNT) {
      return false;
    }
  }

  btnLockout = BTN_LOCKOUT_COUNT;
  return true;
}

void SendButton() {

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    String url = CALL_URL ;
    sendURL(url);
  } else {
    DBG_PRINT( F("[HTTP] call error") );
  }

}


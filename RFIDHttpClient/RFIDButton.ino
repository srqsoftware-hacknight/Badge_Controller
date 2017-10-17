

// 'call' button for doorbell, use internal pull-up resistor (active low)


#define BTN_PIN    D3  // button pin here, tie putton to this pin and GND. Add 10 or 100nF to help debounce.

//#define BTN_COUNT    10 // count required for debounce, (hold button for about 1/2 second to ring)
#define BTN_COUNT    5 // count required for debounce, (hold button for a little while to ring)

#define BTN_LOCKOUT  800 // count required until btn is serviced again (lockout after call)
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
    if (++btnCount < BTN_COUNT) {
      return false;
    }
  }

  btnLockout = BTN_LOCKOUT;
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


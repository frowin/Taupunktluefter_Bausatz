# Changelog

## 7.2.10 - 2026-08-11
- Fix: `processor()` returned empty string for every page placeholder (Optionen table, Speicher, Fehler, page footer) whenever `use_Charts` was off (the default), making Optionen/Speicher/Fehler pages render almost blank. The `use_Charts` guard now only applies to chart data placeholders.

## 7.2.9 - 2026-08-11
- Fix: raise 433 MHz outside-reading freshness window from 30s to 120s. TFA 30.3249.02/30.3250 only transmit every ~50-60s, so 30s caused "Fehler Sensor 2" every cycle even with good reception.

## 7.2.8 - 2026-08-10
- Label TFA sources in Options: 30.3250 (nur Temp), 30.3249.02 (Temp+Feuchte)

## 7.2.7 - 2026-08-10
- Add 433 MHz TFA Dostmann 30.3249.02 (LaCrosse TX141TH-Bv2) outdoor source with humidity
- Decode 40-bit frames with `lfsr_digest8_reflect` CRC; keep 30.3250 as temp-only option
- Use TFA humidity for outdoor dew point when 30.3249.02 is selected

## 7.2.6 - 2026-08-08
- Handle DHT22 and 433 MHz outside temperature modes separately during sensor validation.
- Require selected 433 MHz outside temperature readings to be no older than 30 seconds.

## 7.2.5 - 2026-08-08
- Move the 433 MHz WL-101-341 receiver input from GPIO14 to GPIO16 to avoid the Ampel LED pins.

## 7.2.4 - 2026-08-08
- Move the 433 MHz WL-101-341 receiver input from GPIO25 to GPIO14.
- Note that GPIO14 is also used by Ampel LED2 when the Ampel feature is enabled.

## 7.2.3 - 2026-08-08
- Filter impossible 433 MHz pulse widths in the ISR before they fill the RF ring buffer.
- Drain more queued RF pulses per loop pass and show ignored RF noise in the status text.

## 7.2.2 - 2026-08-08
- Replace the 433 MHz pulse copy buffer with an ISR ring buffer to reduce dropped TFA/Acurite packets.
- Increase RF edge buffering and drain pulses in bounded loop batches.

## 7.2.1 - 2026-08-08
- Keep partially collected TFA 30.3250 bursts across RF buffer overflow instead of clearing them before decode.
- Increase RF pulse buffering and ignore sub-100us noise to reduce missed TFA updates.

## 7.2.0 - 2026-08-08
- Add 433 MHz TFA Dostmann 30.3250 / LaCrosse-TX141Bv3 outdoor temperature source.
- Select TFA channel on Options page: sensor dial 1→CH2, 2→CH3, 3→CH4 (saved like Acurite ID).
- Lower RF ISR pulse filter so short TFA PWM marks are captured alongside Acurite gaps.

## 7.1.2 - 2026-08-08
- Extend valid 433 MHz temperature age to 15 minutes to avoid quick fallback to `nan`.
- Filter short RF pulses before buffering to reduce WL-101-341 noise overflow.
- Show RF overflow and ignored-short-pulse counters in the 433 MHz status.

## 7.1.1 - 2026-08-08
- Accept 433 MHz sensor IDs as hex or decimal and default Acurite 606TX to `0x61`.
- Relax Acurite 606TX repeat filtering for WL-101-341 reception.
- Show RF status with sensor ID, raw frame, age, channel, battery state, and packet count.

## 7.1.0 - 2026-08-08
- Add selectable outside temperature source for DHT22 or 433 MHz Acurite 606TX.
- Add WL-101-341 receiver support on GPIO25 with saved sensor ID, defaulting to 61.
- Add `/Optionen` website controls for outside temperature source and 433 MHz sensor ID.
- Keep outdoor DHT22 humidity active for dew point calculation when 433 MHz temperature is selected.

# Security Instructions

## Privacy Architecture

- **Default Policy:** Zero external send policy.
- **Audit Command:** `audit-self` (show active network connections, files accessed).
- **Plugin Sandbox:** Memory bounds enforced, IO restricted.

## Security Measures

- **System Resource Tracker:** CPU, RAM, Temp, Disk IO.
- **Syscall Monitor:** Especially for plugin behavior.
- **Clipboard Monitor (optional, user-enabled).**

## Additional Notes

- Total overhead <1% CPU at idle.
- Ensure all external connections are secure and monitored.
- Regularly audit system for any unauthorized access or anomalies.

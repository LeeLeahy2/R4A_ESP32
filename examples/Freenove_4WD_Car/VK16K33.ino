/**********************************************************************
  VK16K33.ino

  LED Matrix support using the VK16K33
**********************************************************************/

//*********************************************************************
// Display the delta time on VK16K33
void vk16k33DeltaTime(uint32_t milliseconds)
{
    uint32_t thousands;
    uint32_t hundreds;
    uint32_t seconds;
    uint32_t tens;
    uint32_t tenths;

    // Parse the time
    seconds = milliseconds / R4A_MILLISECONDS_IN_A_SECOND;
    milliseconds -= seconds * R4A_MILLISECONDS_IN_A_SECOND;
    thousands = seconds / 1000;
    seconds -= thousands * 1000;
    hundreds = seconds / 100;
    seconds -= hundreds * 100;
    tens = seconds / 10;
    seconds -= tens * 10;
    tenths = milliseconds / 100;

    // Handle overflow
    if (thousands > 1)
    {
        thousands = ' ';
        hundreds = '*';
        tens = '*';
        seconds = '*';
    }
    else
    {
        // Handle leading zero suppression
        if (thousands)
        {
            thousands += '0';
            hundreds += '0';
            tens += '0';
        }
        else if (hundreds)
        {
            thousands = ' ';
            hundreds += '0';
            tens += '0';
        }
        else if (tens)
            tens += '0';
        seconds += '0';
        tenths += '0';
    }

    // Display the time with zero suppression
    r4aVk16k33BufferClear(&vk16k33);
    if (hundreds)
    {
        // Display the range 100 - 1999 or overflow
        r4aVk16k33DisplayChar(&vk16k33, 0, (char)thousands);
        r4aVk16k33DisplayChar(&vk16k33, 1, (char)hundreds);
        r4aVk16k33DisplayChar(&vk16k33, 6, (char)tens);
        r4aVk16k33DisplayChar(&vk16k33, 11, (char)seconds);
    }
    else
    {
        // Display the range 0 - 99.9
        if (tens)
            r4aVk16k33DisplayChar(&vk16k33, 0, (char)tens);
        r4aVk16k33DisplayChar(&vk16k33, 5, (char)seconds);
        r4aVk16k33DisplayChar(&vk16k33, 10, '.');
        r4aVk16k33DisplayChar(&vk16k33, 11, (char)tenths);
    }
    r4aVk16k33DisplayPixels(&vk16k33);
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DXF
{
    /// <summary>
    /// Provides methods to convert a string to a double precision floating
    /// point number.
    /// </summary>
    public static class StringToDouble
    {
        /// <summary>
        /// Extracts a number from the specified string, starting at the
        /// specified index.
        /// </summary>
        /// <param name="input">The string to extract a number from.</param>
        /// <param name="start">
        /// The index of the first character in the string to start converting
        /// to a number.
        /// </param>
        /// <param name="end">
        /// When this methods returns, contains the index of the end of the
        /// extracted number.
        /// </param>
        /// <returns>A number representation of the string.</returns>
        /// <exception cout="ArgumentNullException">input is null.</exception>
        /// <exception cout="ArgumentOutOfRangeException">
        /// start represents an index that is outside of the range for input.
        /// </exception>
        public static double Parse(string input, int start, out int end)
        {
            if (input == null)
            {
                throw new ArgumentNullException("input");
            }
            if (start < 0)
            {
                throw new ArgumentOutOfRangeException("start", "Value cannot be negative.");
            }
            if (start > input.Length)
            {
                throw new ArgumentOutOfRangeException("start", "Value must be less then input.Length");
            }

            int endOfWhitespace = SkipWhiteSpace(input, start);

            long significand;
            int significandsExponent, sign;
            int startOfDigits = ParseSign(input, endOfWhitespace, out sign);
            int index = SkipLeadingZeros(input, startOfDigits);
            index = ParseSignificand(input, index, out significand, out significandsExponent);

            // Have we parsed a number?
            if ((index - startOfDigits) > 0)
            {
                int exponent;
                end = ParseExponent(input, index, out exponent);
                return MakeDouble(significand * sign, exponent - significandsExponent);
            }

            // Not a number, is it a constant?
            double value;
            end = ParseNamedConstant(input, endOfWhitespace, out value);
            if (end != endOfWhitespace)
            {
                return value;
            }

            // If we're here then we couldn't parse anything.
            end = start;
            return default(double);
        }

        private static double MakeDouble(long significand, int exponent)
        {
            // Improve the accuracy of the result for negative exponents.
            if (exponent < 0)
            {
                // Allow for denormalized numbers (allows values less than emin)
                if (exponent < -308) // Smallest normalized floating points number
                {
                    return (significand / Math.Pow(10, -308 - exponent)) / 1e308;
                }
                return significand / Math.Pow(10, -exponent);
            }
            return significand * Math.Pow(10, exponent);
        }

        private static int ParseExponent(string str, int startIndex, out int exponent)
        {
            exponent = 0;
            int index = startIndex;
            if (index < str.Length)
            {
                if ((str[index] == 'e') || (str[index] == 'E'))
                {
                    int sign;
                    index = ParseSign(str, index + 1, out sign); // Add one to the index to skip the 'e'

                    int digitStart = index; // Keep a track of if we parse any digits
                    index = SkipLeadingZeros(str, index);

                    long value = 0;
                    index = ParseNumber(3, str, index, out value);
                    if ((index - digitStart) == 0)
                    {
                        exponent = 0;
                        return startIndex; // We didn't parse anything
                    }

                    exponent = (int)(value * sign);
                }
            }
            return index;
        }

        private static int ParseNamedConstant(string str, int startIndex, out double value)
        {
            // StartsWith only converts str to uppercase - important we pass the constant in uppercase
            if (StartsWith(str, startIndex, "NAN"))
            {
                value = double.NaN;
                return startIndex + 3;
            }

            // Infinity can be positive or negative
            int sign;
            int index = ParseSign(str, startIndex, out sign);
            if (StartsWith(str, index, "INFINITY"))
            {
                value = sign * double.PositiveInfinity;
                return index + 8;
            }
            if (StartsWith(str, index, "INF"))
            {
                value = sign * double.PositiveInfinity;
                return index + 3;
            }

            // No match
            value = default(double);
            return startIndex;
        }

        private static int ParseNumber(int maxDigits, string str, int index, out long value)
        {
            value = 0;
            for (; index < str.Length; index++)
            {
                char c = str[index];
                if ((c < '0') || (c > '9'))
                {
                    break;
                }
                if (--maxDigits >= 0)
                {
                    value = (value * 10) + (c - '0');
                }
            }
            return index;
        }

        private static int ParseSign(string str, int index, out int value)
        {
            value = 1; // Default to positive
            if (index < str.Length)
            {
                char c = str[index];
                if (c == '-')
                {
                    value = -1;
                    return index + 1;
                }

                if (c == '+')
                {
                    return index + 1;
                }
            }
            return index;
        }

        private static int ParseSignificand(string str, int startIndex, out long significand, out int exponent)
        {
            exponent = 0;
            significand = 0;
            int index = ParseNumber(18, str, startIndex, out significand);
            int digits = index - startIndex;

            // Is there a decimal part as well?
            if ((index < str.Length) && (str[index] == '.'))
            {
                int point = ++index; // Skip the decimal point

                // If there are no significant digits before the decimal point
                // then skip the zeros after it as well, making sure we adjust
                // the exponent.
                // e.g. .0001 == 1e-4 (-3 zeros + -1 decimal digit == -4)
                if (digits == 0)
                {
                    index = SkipLeadingZeros(str, index);
                }

                index = ParseNumber(18 - digits, str, index, out significand);
                exponent = index - point;

                // Check it's not just a decimal point
                if ((index - startIndex) == 1)
                {
                    return startIndex; // Didn't parse anything
                }
            }

            return index;
        }

        private static int SkipLeadingZeros(string str, int index)
        {
            for (; index < str.Length; index++)
            {
                if (str[index] != '0')
                {
                    break;
                }
            }
            return index;
        }

        private static int SkipWhiteSpace(string str, int index)
        {
            for (; index < str.Length; index++)
            {
                char c = str[index];
                if ((c != '\x09') &&
                    (c != '\x0D') &&
                    (c != '\x0A') &&
                    (c != '\x20'))
                {
                    break;
                }
            }
            return index;
        }

        private static bool StartsWith(string str, int startIndex, string value)
        {
            for (int i = 0; i < value.Length; i++)
            {
                if ((startIndex + i) >= str.Length)
                {
                    return false;
                }
                if (char.ToUpperInvariant(str[startIndex + i]) != value[i])
                {
                    return false;
                }
            }
            return true;
        }
    }
}

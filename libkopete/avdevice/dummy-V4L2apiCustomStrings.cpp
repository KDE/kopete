/*
    dummy-V4L2apiCustomStrings.cpp  -  Dummy cpp file used as string catalog for translation

    Copyright (c) 2010 by Frank Schaefer <fschaefer.oss@googlemail.com>

    Kopete    (c) 2010 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    * This program is distributed in the hope that it will be useful,       *
    * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
    * GNU General Public License for more details.                          *
    *                                                                       *
    * You should have received a copy of the GNU General Public License     *
    * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
    *************************************************************************
*/

/* NOTE: this is a dummy cpp-file. It is only used for translation purposes and must be ignored during the build process.
         The .cpp is necessary to make sure that the l10n string extraction tools work properly
 */

// --- V4L2 API custom (driver defined) strings from kernel sources (driver/media/video/) to make them translatable ---
// Last update: kernel 2.6.33

// Custom video control titles:
I18N_NOOP("Target KB");				// cpia2/cpia2_v4l.c
I18N_NOOP("GPIO");				// cpia2/cpia2_v4l.c
I18N_NOOP("Flicker Reduction");			// cpia2/cpia2_v4l.c
I18N_NOOP("Framerate");				// cpia2/cpia2_v4l.c
I18N_NOOP("USB Alternate");			// cpia2/cpia2_v4l.c
I18N_NOOP("Lights");				// cpia2/cpia2_v4l.c
I18N_NOOP("Reset Camera");			// cpia2/cpia2_v4l.c
I18N_NOOP("chroma agc");			// bt8xx/bttv-driver.c
I18N_NOOP("combfilter");			// bt8xx/bttv-driver.c
I18N_NOOP("automute");				/* bt8xx/bttv-driver.c,
						   saa7134/saa7134-video.c */
I18N_NOOP("luma decimation filter");		// bt8xx/bttv-driver.c
I18N_NOOP("agc crush");				// bt8xx/bttv-driver.c
I18N_NOOP("vcr hack");				// bt8xx/bttv-driver.c
I18N_NOOP("whitecrush upper");			// bt8xx/bttv-driver.c
I18N_NOOP("whitecrush lower");			// bt8xx/bttv-driver.c
I18N_NOOP("uv ratio");				// bt8xx/bttv-driver.c
I18N_NOOP("full luma range");			// bt8xx/bttv-driver.c
I18N_NOOP("coring");				// bt8xx/bttv-driver.c
I18N_NOOP("Invert");				// saa7134/saa7134-video.c
I18N_NOOP("y offset odd field");		// saa7134/saa7134-video.c
I18N_NOOP("y offset even field");		// saa7134/saa7134-video.c
I18N_NOOP("green balance");			/* zc0301/zc0301_pas202bcb.c,
						   sn9c102/sn9c102_pas202bcb.c,
						   sn9c102/sn9c102_pas106b.c,
						   sn9c102/sn9c102_ov7660.c,
						   sn9c102/sn9c102_ov7630.c,
						   sn9c102/sn9c102_mi0360.c,
						   sn9c102/sn9c102_mi0343.c,
						   sn9c102/sn9c102_hv7131.c,
						   sn9c102/sn9c102_hv7131d.c,
						   gspca/m5602/m5602_po1030.c,
						   gspca/m5602/m5602_mt9m111.c */
I18N_NOOP("DAC magnitude");			/* zc0301/zc0301_pas202bcb.c,
						   sn9c102/sn9c102_pas202bcb.c,
						   sn9c102/sn9c102_pas106b.c */
I18N_NOOP("band filter");			/* sn9c102/sn9c102_ov7660.c,
						   sn9c102/sn9c102_ov7630.c */
I18N_NOOP("rgb gamma");				// sn9c102/sn9c102_ov7630.c.
I18N_NOOP("reset level");			// sn9c102/sn9c102_hv7131d.c
I18N_NOOP("pixel bias voltage");		// sn9c102/sn9c102_hv7131d.c
I18N_NOOP("Noise suppression (smoothing)");	// gspca/m5602/m5602_s5k4aa.c
I18N_NOOP("Minimum Clock Divider");		// gspca/mr97310a.c
I18N_NOOP("Webcam Effects");			// gspca/t613.c
I18N_NOOP("Infrared");				// gspca/sonixj.c
I18N_NOOP("B/W");				// hexium_gemini.c
I18N_NOOP("Auto Luminance Control");		// tcm825x.c
I18N_NOOP("Horizontal Edge Enhancement");	// tcm825x.c
I18N_NOOP("Vertical Edge Enhancement");		// tcm825x.c
I18N_NOOP("Lens Shading Compensation");		// tcm825x.c
I18N_NOOP("Maximum Exposure Time");		// tcm825x.c
I18N_NOOP("Red Saturation");			// vino.c (=> indycam.h)
I18N_NOOP("Blue Saturation");			// vino.c (=> indycam.h)
I18N_NOOP("Luminance Bandpass");		// vino.c (=> ssa7191.c)
I18N_NOOP("Luminance Bandpass Weight");		// vino.c (=> ssa7191.c)
I18N_NOOP("HF Luminance Coring");		// vino.c (=> ssa7191.c)
I18N_NOOP("Force Colour");			// vino.c (=> ssa7191.c)
I18N_NOOP("Chrominance Gain Control");		// vino.c (=> ssa7191.c)
I18N_NOOP("VTR Time Constant");			// vino.c (=> ssa7191.c)
I18N_NOOP("Luminance Delay Compensation");	// vino.c (=> ssa7191.c)
I18N_NOOP("Vertical Noise Reduction");		// vino.c (=> ssa7191.c)
I18N_NOOP("Save User Settings");		// pwc/pwc_v4l.c
I18N_NOOP("Restore User Settings");		// pwc/pwc_v4l.c
I18N_NOOP("Restore Factory Settings");		// pwc/pwc_v4l.c
I18N_NOOP("Colour mode");			// pwc/pwc_v4l.c
I18N_NOOP("Auto contour");			// pwc/pwc_v4l.c
I18N_NOOP("Contour");				// pwc/pwc_v4l.c
I18N_NOOP("Backlight compensation");		// pwc/pwc_v4l.c
I18N_NOOP("Flickerless");			// pwc/pwc_v4l.c
I18N_NOOP("Noise reduction");			// pwc/pwc_v4l.c


// Option strings for video control V4L2_CID_POWER_LINE_FREQUENCY:
I18N_NOOP("NoFliker");
I18N_NOOP("Automatic");
// also: 50 Hz, 60 Hz


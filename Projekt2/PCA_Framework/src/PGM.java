import java.io.IOException;
import java.io.InputStream;

import org.jblas.DoubleMatrix;

/**
 * Skeleton class to encapsulate the PGM images. 
 * This class should save meta data about the image, such as width and height, 
 * and also create an array of pixels, which is used to create the DoubleMatrix
 * used to do the PCA calculation. 
 * The path attribute is also good to save, so you have a chance to map the .pgm
 * image back to the original File object. 
 * 
 * This is only a skeleton class. You can choose to use a constructor with a 
 * different signature as well, or not use this class at all. It is only here
 * to give guidance. 
 * 
 * @author Fabian Kammel <fabian.kammel@rub.de>
 * @author Jan Rimkus <jan.rimkus@rub.de>
 */

public class PGM  implements Comparable<PGM>{
	
	public int width;
	public int height;
	private double[] pixels;
	private DoubleMatrix y;
	private String path;
	public double distanceToTest;
	
	
	public PGM(InputStream in, String path) {
		
		/* Save path for later */
		this.path = path;

		/* Check Magic Bytes and read width and height, and calculate
		 * Array Size */
		char c = 'x';
		try {
			// Skip magicByte
			String magicByte = "P5"; 
			while (!isWhiteSpace(c)) {
				c = (char)in.read();
			}
			c = (char)in.read();
			
			//Get width
			String widthTmp = ""; 
			while (!isWhiteSpace(c)) {
				widthTmp = widthTmp + c;
				c = (char)in.read();
			}
			this.width = Integer.parseInt(widthTmp);
			c = (char)in.read();
			
			
			//Get height 
			String heightTmp = ""; 
			while (!isWhiteSpace(c)) {
				heightTmp = heightTmp + c;
				c = (char)in.read();
			}
			this.height = Integer.parseInt(heightTmp);
			c = (char)in.read();
			
			// Skip max value
			int maxVal= 255; 
			while (!isWhiteSpace(c)) {
				c = (char)in.read();
			}

			//Calculate and read the pixels
			this.pixels = new double[width * height];
			for (int j = 0; j < pixels.length; j++) {
				double d = (double)in.read();
				this.pixels[j] = d;
			}
			
		} catch (IOException e) {
			//in.read didnt work
			e.printStackTrace();
			this.width = 0;
			this.height = 0;
			this.pixels = new double[width * height];
		}

		
		/* Create DoubleMatrix by reading from the File/InputStream */
		y = new DoubleMatrix(this.pixels);
		
	}
	
	/* Some other helper methods you can choose to use, or delete */
	
	public double getElement(int pos) {
		if((pos < pixels.length) && (pos >= 0)) {
			return this.pixels[pos];
		}
		return Double.NaN;
	}
	
	public double[] getPixels() { 
		return (double[]) this.pixels.clone();
	}
	
	public DoubleMatrix getMatrix() {
		return new DoubleMatrix(this.pixels.clone());
	}
	
	public String getPath() {
		return path;
	}
	
	private static boolean isWhiteSpace(int c) {
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}

	public int compareTo(PGM comparePgm) {
		double compareDistanze = ((PGM) comparePgm).distanceToTest;

		if (this.distanceToTest - compareDistanze > 0) {
			return 1;
		} else {
			return -1;
		}
	}
}

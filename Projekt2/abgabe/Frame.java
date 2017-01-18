import java.awt.Color;
import java.awt.Dimension;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.GridLayout;
import java.awt.Image;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 * THIS IS A GUI FOR THE PCA FACE RECOGNITION
 * @author Fabian Kammel <fabian.kammel@rub.de>
 * @author Jan Rimkus <jan.rimkus@rub.de>
 * 
 * !!!!!!!! DO NOT CHANGE ANYTHING HERE !!!!!!!
 * 
 * GO TO THE PCA CLASS AND SUPPLY YOUR IMPLEMENTATION
 * IN THE doPCA() FUNCTION!
 *
 */
public class Frame extends JFrame {
	private static final long serialVersionUID = 1L;
	// ID Generator for Images
	private static int imgCnt = 0;
	
	/* Needed to get a subset of images
	 * We compute 5 distinct numbers and use them
	 * as indicies to load images
	 * N = Amount of Images
	 * K = cardinality of the subset
	 * S = Set of images
	 * C = Subsect of images (combination)
	 */
	private static final int N = 400; //TODO: Put number of images in here
	private static final int doubleOcc = 10; //TODO: Put number of images per person here
	private static final int K = 5; //TODO: Put number of images in subset here
	private static final List<Integer> S = new ArrayList<Integer>(N);
	static {
		//Only take the first of 10 (doubleOcc) images so we get only the first image of every person
		for(int i = 0; i < N; i+=doubleOcc) { 
			S.add(i + 1);
		}
	}
	private static final List<Integer> C = new ArrayList<Integer>(K);
	
	/**
	 * This formatter formats a double so that it has exactly two decimal places.
	 */
	private static final DecimalFormat DEC_FORMAT = new DecimalFormat("#.00");

	//Path to images
	private static final File dir = new File("faces");
	
	//Accepted extensions
	private static final String[] EXTENSIONS = new String[] {"pgm"};
	//Filter for .pgm, so we only load the images/files we want
	private static final FilenameFilter IMAGE_FILTER = new FilenameFilter() {
		
		public boolean accept(File dir, String name) {
			for(final String ext : EXTENSIONS) {
				if (name.endsWith("." + ext)) {
					return true;
				}
			}
			return false;
		}
	};
	
	private static final String[] EXTENSIONS_PNG = new String[] {"png"};
	//Filter for .png, so we only load the images/files we want
	private static final FilenameFilter IMAGE_FILTER_PNG = new FilenameFilter() {
		
		public boolean accept(File dir, String name) {
			for(final String ext : EXTENSIONS_PNG) {
				if (name.endsWith("." + ext)) {
					return true;
				}
			}
			return false;
		}
	};
	
	/* Private static objects */
	private static int lastClicked = 0;
	
	/* Private objects */
	private List<JLabel> neededLabels;
	private List<File> files;
	private List<Image> images;
	private Image unknownImage;
	private PCA pca;
	
	/**
	 * Factory function to create the Panels more easily
	 * @param img Is the image which is used as the background for the panel
	 * @param neededLabels List of Labels where we need a reference to later on
	 * @param toAdd Shall this panel be added to the neededLabels list? 
	 * @param clickAble Shall this panel be select-able as the to find image?
	 * @return The created JPanel
	 */
	private static JComponent createPanel(Image img, List<JLabel> neededLabels, boolean toAdd, boolean clickAble) {
		/* Panel where the images will be shown */
		JPanel p = new JPanel();
		p.setSize(50, 50);
		
		/* JLabel that is used to display the image */
		JLabel picLabel = null;
		if(img == null) {
			System.err.print("Cannot createPanel, supplied image object is null!");
			System.exit(-1);
		} else {
			picLabel = new JLabel(new ImageIcon(img));
		}
		
		/* Border to display currently selected Image */
		picLabel.setBorder(BorderFactory.createLineBorder(Color.WHITE, 2));
		final List<JLabel> otherLabels = neededLabels;
		final boolean clickAbleFinal = clickAble;
		if(toAdd) {
			picLabel.setText("" + imgCnt++);
		}
		picLabel.addMouseListener(new MouseListener() {
			
			@Override
			public void mouseReleased(MouseEvent arg0) {
				//NOT NEEDED
			}
			
			@Override
			public void mousePressed(MouseEvent arg0) {
				if(clickAbleFinal) {
					JLabel label = (JLabel)arg0.getComponent();
					lastClicked = Integer.parseInt(label.getText());
					System.out.println("Mouse Clicked in " + label.getText());
					
					/* Border to display currently selected Image */
					Iterator<JLabel> it = otherLabels.iterator();
					while(it.hasNext()) {
						it.next().setBorder(BorderFactory.createLineBorder(Color.WHITE, 2));
					}
					label.setBorder(BorderFactory.createLineBorder(Color.BLUE, 2));
				}
			}
			
			@Override
			public void mouseExited(MouseEvent arg0) {
				//NOT NEEDED
			}
			
			@Override
			public void mouseEntered(MouseEvent arg0) {
				//NOT NEEDED
			}
			
			@Override
			public void mouseClicked(MouseEvent arg0) {
				//NOT NEEDED
			}
			
		});
		//Only add the upper images. And the result images. Not the while filler images 
		if(toAdd) {
			neededLabels.add(picLabel);
		}
		
		
		p.add(picLabel);
		p.repaint();
		
		return p;
	}
	
	/**
	 * Factory function to create Buttons more easily. 
	 * @param s Text inside the JButton object
	 * @param al ActionListener coupled with the button
	 * @return The created JButton
	 */
	private static JComponent createButton(String s, ActionListener al) {
		JButton b = new JButton();
		b.setText(s);
		b.addActionListener(al);
		
		return b;
	}
	
	public Frame() {
		
		//Load all the images into memory
		loadImages();
		
		/*
		 * Used to have a reference to the Labels that have to be accessed in at a later point
		 * neededLabels 0...4 contain the upper images which have to be shuffled
		 * neededLabels 5...7 contain the three result images, where the placeholder images
		 * has to be replaced with the actual result images
		 */
		neededLabels = new ArrayList<JLabel>(20);
		
		/* 
		 * Top Panel for the Images
		 */
		JPanel imagePanel = new JPanel(new GridLayout(0, 5));
		imagePanel.setSize(600, 100);
		imagePanel.setBackground(Color.WHITE);
		
		// Add 5 random images to the upper Panel
		shuffleC();
		for(int i = 0; i < 5; i++) {
			imagePanel.add(createPanel(images.get(C.get(i)), neededLabels, true, true));
		}

		/*
		 * Middle Panel for the buttons
		 */
		JPanel buttonPanel = new JPanel(new GridLayout(0, 2));
		buttonPanel.setSize(600, 100);
		buttonPanel.setBackground(Color.WHITE);
		
		buttonPanel.add(createButton("Shuffle", new SuffleListener()));
		buttonPanel.add(createButton("Calculate", new CalcListener()));

		
		/*
		 * Bottom Panel for the results
		 */
		JPanel resultPanel = new JPanel(new GridLayout(0, 3));
		resultPanel.setSize(600, 100);
		resultPanel.setBackground(Color.WHITE);
		
		//resultPanel.add(createPanel(whiteImage, neededLabels, false, false));
		resultPanel.add(createPanel(unknownImage, neededLabels, true, false));
		resultPanel.add(createPanel(unknownImage, neededLabels, true, false));
		resultPanel.add(createPanel(unknownImage, neededLabels, true, false));
		//resultPanel.add(createPanel(whiteImage, neededLabels, false, false));
	
		
		//Set meta data of the main panel
		Dimension size = new Dimension(600, 400);
		this.setSize(size);
		this.setPreferredSize(new Dimension(size));
		this.setMaximumSize(new Dimension(size));
		this.setMinimumSize(new Dimension(size));
		this.setResizable(false);
		this.setLocationRelativeTo(null);
		this.setBackground(Color.WHITE);
		this.setName("PCA-based Face Recognition");
		this.setTitle("PCA-based Face Recognition");
		this.setAutoRequestFocus(true);
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLayout(new GridLayout(3, 0));
		
		//Add the three panels to the main panel
		this.add(imagePanel);
		this.add(buttonPanel);
		this.add(resultPanel);
		
		//Display everything
		this.setVisible(true);
		this.pack();
		
	}
	
	/**
	 * Used in the constructor of this class. 
	 * It loads all the image necessary to run this tool. 
	 * the ArrayList file is created and loaded with File objects
	 * to the *.pgm images
	 * the ArrayList images is created and loaded with Image objects
	 * to the *.png images
	 * also a white image is loaded to clear the bottom left and right picture
	 * and a unknown image is loaded as a placeholder as long as no results
	 * are returned 
	 */
	private void loadImages() {
		files = new ArrayList<File>(20);
		images = new ArrayList<Image>(20);
		Image image = null;
		BufferedImage bImage = null;
		GraphicsConfiguration gc = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice().getDefaultConfiguration();
		
		if(dir.isDirectory()) { //test if is really a directory
			
			//Load all .pgm into File List
			File[] listOfFiles = dir.listFiles(IMAGE_FILTER);
			Arrays.sort(listOfFiles);
			for (final File f : listOfFiles) {
				try {
					files.add(f);
										
					System.out.println("Successfully loaded file: " + f.getName());
					
				} catch (final Exception e) {
					e.printStackTrace();
				}
			}
			
			//Load all .png into Image/BufferedImage List
			listOfFiles = dir.listFiles(IMAGE_FILTER_PNG);
			Arrays.sort(listOfFiles);
			for (final File f : listOfFiles) {
				try {
					bImage = ImageIO.read(f);
					image = gc.createCompatibleImage(bImage.getWidth(), bImage.getHeight());
					image.getGraphics().drawImage(bImage, 0, 0, null);
					images.add(image);
					
					System.out.println("Successfully loaded image: " + f.getName() + " " + image.getHeight(null));
					
				} catch (final Exception e) {
					e.printStackTrace();
				}
			}
			
		} else { // not a directory
			System.err.println("Can't find the specified directory!");
			System.exit(-1);
		}
		
		//Check if enough files were loaded
		if(files.size() < 5) {
			System.err.println("File List has too few Files: " + files.size());
			System.exit(-1);
		}
		
		//Check if enough images were loaded
		if(images.size() < 5) {
			System.err.println("Image List has too few Images: " + images.size());
			System.exit(-1);
		}

		//Load white Image and unknown Image
		try {
			File f = null;
			BufferedImage bi = null;
			
			f = new File(dir + File.separator + "unknown.png");
			bi = ImageIO.read(f);
			unknownImage = gc.createCompatibleImage(bi.getWidth(), bi.getHeight());
			unknownImage.getGraphics().drawImage(bi, 0, 0, null);
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}
	
	/**
	 * Helper function which is used to load images by name. 
	 * The directory is the same for all images and can be manipulated
	 * in the top of this class in the 'dir' variable. 
	 * 
	 * @param name is the name of the image that is loaded
	 * @return an Image object of the loaded image
	 */
	private Image loadImageByName(String name) {
		Image result = null;
		
		GraphicsConfiguration gc = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice().getDefaultConfiguration();
		File f = new File(dir + File.separator + name);

		try {
			BufferedImage bi = ImageIO.read(f);
			result = gc.createCompatibleImage(bi.getWidth(), bi.getHeight());
			result.getGraphics().drawImage(bi, 0, 0, null);
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		return result;
	}
	
	/**
	 * This method is used to shuffle the set (S) of all indexes
	 * with images that can be displayed. And then write the subset
	 * of the lower (K) images to the subset (C). 
	 */
	private static void shuffleC() {
		Random rnd = new Random(System.currentTimeMillis());
		Collections.shuffle(S, rnd);
		C.clear();
		C.addAll(S.subList(0, K));
	}
	
	/**
	 * Is called when the shuffle button is pressed. 
	 * This class uses a list of all indexes of the images
	 * that are allowed to be displayed shuffles them and
	 * uses the subset of the first five indexes to display
	 * random images without double occurrences. 
	 */
	public class SuffleListener implements ActionListener {

		@Override
		public void actionPerformed(ActionEvent arg0) {
			System.out.println("Shuffle");
			
			shuffleC();
			
			for(int i = 0; i < 5; i++) {
				neededLabels.get(i).setIcon(new ImageIcon(images.get(C.get(i))));
			}
		}
	}
	
	/**
	 * Is called when the calculate button is pressed
	 * This class creates a new PCA object and passes
	 * the file list (without the to find image) to the
	 * PCA object and displays the result afterwards
	 */
	public class CalcListener implements ActionListener {
		@Override
		public void actionPerformed(ActionEvent arg0) {
			File toTest = files.get(C.get(lastClicked));
			System.out.println("You have selected "+toTest.getName()+" as probe image");
			
			//Temporarily remove image, so we do not get a 100% match
			int oldPos = files.indexOf(toTest);
			files.remove(toTest);
			
			//Calculate the PCA and set the result Lists
			pca = new PCA(files);
			pca.doPCA(toTest);
			
			//Readd the temp. removed image
			files.add(oldPos, toTest);
			
			//Get the results
			List<File> resultFiles = pca.getResultFiles();
			List<Double> resultProbs = pca.getResultDist();
			
			//Display the results
			for(int i = 0; i < 3; i++) {
				String name = resultFiles.get(i).getName();
				// Replace PGM with PNG to display the results
				if (name.endsWith("pgm")) {
                    name = name.substring(0, name.length() - "pgm".length());
                    name = name + "png";
                }
				neededLabels.get(i+5).setIcon(new ImageIcon(loadImageByName(name)));
				if (name.endsWith("png")) {
                    name = name.substring(0, name.length() - "png".length());
                    name = name + "pgm";
                }
				final String distance = DEC_FORMAT.format(resultProbs.get(i));
				System.out.println("A similar image to "+toTest.getName()+" is "+name+", which got a distance of "+distance);
				neededLabels.get(i+5).setText(distance);
			}
		}
	}
	
	public static void main(String[] args) {
		
		new Frame();
	}

}

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.ObjectOutputStream.PutField;
import java.util.ArrayList;
import java.util.List;

import org.jblas.ComplexDoubleMatrix;
import org.jblas.DoubleMatrix;
import org.jblas.Eigen;

/**
 * 
 * THIS IS A TESTBENCH FOR PCA FACE RECOGNITION
 * @author Fabian Kammel <fabian.kammel@rub.de>
 * @author Jan Rimkus <jan.rimkus@rub.de>
 * 
 * PLEASE SUPPLY YOUR IMPLEMENTATION IN THE doPCA() FUNCTION
 * AND RETURN THE FOUND IMAGES AND PROBABILITIES AS EXPLAINED
 * IN THE FUNCTION DESCRIPTION
 * 
 * THE WHOLE PROJECT MUST COMPILE WITHOUT WARNINGS AND ERRORS 
 * 
 * ANY FURTHER INFORMATION CAN BE FOUND IN THE PROBLEM DESCRIPTION 
 * IF YOU NEED ANY ADDITIONAL HELP REGARDING THE PROGRAMMING
 * ASSIGMNMENT WRITE A MAIL TO:  
 * Maximilian Golla <maximilian.golla@rub.de>
 *
 */

public class PCA {
	
	private List<File> set;
	private List<File> resultFile;
	private List<Double> resultDistance;
	
	public PCA(List<File> set) {
		this.set = set;
		
		this.resultFile = new ArrayList<File>(3);
		this.resultDistance = new ArrayList<Double>(3);
		
	}
	
	public List<File> getResultFiles() {
		return resultFile;
	}
	
	public List<Double> getResultDist() {
		return resultDistance;
	}
	
	private void addFileResult(int index, File f) {
		resultFile.add(index, f);
	}
	
	private void addDistResult(int index, Double d) {
		resultDistance.add(index, d);
	}
	
	/**
	 * Calculate the three most similar pictures from the 'set' of files, compared
	 * to the toTest File, supplied to the function. 
	 * 
	 * Write your results to the resultFile and resultDistance lists using the
	 * function addFileResult() and addDistResult(). Where index 0 is the
	 * file/image with the most similarities to the probe image, and index 2 is 
	 * the file/image with the least similarities to the probe image.
	 * 
	 * resultFile has to contain a File object
	 * resultDist has to contain the euclidean distance to the probe image
	 * 
	 * You can also chose to write directly to the result lists. 
	 * 
	 * Indexes >= 3 are not used in the resultFile and resultDistance lists. 
	 * 
	 * @param toTest The image that has to be compared to the set of files
	 * 
	 */
	public void doPCA(File toTest) {
		
		InputStream in = null;	
		List<PGM> pgmFileList = new ArrayList<PGM>();;
		
		for ( File file : this.set ) {
			try {
				in = new FileInputStream(file.getAbsolutePath());
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}
			pgmFileList.add(new PGM(in, file.getAbsolutePath()));
		}
		
		
		DoubleMatrix mu = new DoubleMatrix(10304); 
		for (PGM pgm : pgmFileList) {
			mu = mu.add(pgm.getMatrix());
		}
		mu = mu.divi(pgmFileList.size());
		
		double[][] x1 = {{ 2.0, 2.0, 1.0}};
		DoubleMatrix x_1 = new DoubleMatrix(x1).transpose();
		double[][] x2 = {{ 0.0, 0.0, 1.0}};
		DoubleMatrix x_2 = new DoubleMatrix(x2).transpose();
		double[][] xProb = {{ 2.0, 1.0, 1.0}};
		DoubleMatrix x = new DoubleMatrix(xProb).transpose();
		
		//DoubleMatrix mu = x_1.add(x_2).muli(0.5);
		//vertically = New matrix will be x atop y. lol wtf
		DoubleMatrix X = DoubleMatrix.concatHorizontally(x_1.sub(mu), x_2.sub(mu));
		
		DoubleMatrix Xt = X.transpose();
		DoubleMatrix D = Xt.mmul(X); //Not optimized version: C = X.mmul(Xt)
		
		/* Eigenvectors and Eigenvalues */
		ComplexDoubleMatrix[] complexE = Eigen.eigenvectors(D);

		DoubleMatrix eVectorsD = DoubleMatrix.zeros(complexE.length, complexE.length);
		for (int i = 0; i < complexE.length; i++) {
			for (int j = 0; j < complexE.length; j++) {
				eVectorsD.put(i, j, complexE[0].get(i, j).real());
			}
		}
		
		//For every eigenvektor of D, calculate the real eigenvector for C 
		DoubleMatrix fi;
		DoubleMatrix eVectorsC = X.mmul(eVectorsD.getColumn(0));
		for (int i = 1; i < eVectorsD.columns; i++) {
			fi = X.mmul(eVectorsD.getColumn(i));
			eVectorsC = DoubleMatrix.concatHorizontally(eVectorsC, fi);
		}
		
//		DoubleMatrix eValues = DoubleMatrix.zeros(complexE.length +1, complexE.length +1);
//		for (int i = 0; i < complexE.length; i++) {
//			for (int j = 0; j < complexE.length; j++) {
//			eValues.put(i, j, complexE[1].get(i, j).real());
//			}
//		}
		
		//Compute Featurevectors
		DoubleMatrix eVectorsCT = eVectorsC.transpose();
		DoubleMatrix y1 = eVectorsCT.mmul(x_1);
		DoubleMatrix y2 = eVectorsCT.mmul(x_2);
		DoubleMatrix y = eVectorsCT.mmul(x);
		
		//Compute Distanze
		double d1 = y1.distance2(y);
		double d2 = y2.distance2(y);
		
		//This is how you add files to the result
		addFileResult(0, toTest);
		addFileResult(1, toTest);
		addFileResult(2, toTest);
		
		//This is how you add the corresponding probs to the result
		addDistResult(0, 100.0);
		addDistResult(1, 90.0);
		addDistResult(2, 80.0);
		
		/* ------------------------------------- */
		/* DELETE THIS AND FILL IN YOUR OWN CODE */
		
		return;
	}

}

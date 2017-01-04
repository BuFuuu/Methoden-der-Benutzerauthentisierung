import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.ObjectOutputStream.PutField;
import java.util.ArrayList;
import java.util.Collections;
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
		
		InputStream in = null, inTest = null;	
		List<PGM> pgmFileList = new ArrayList<PGM>();
		
		PGM toTestPgm = readToTestFile(toTest, inTest);
		readAllFiles(in, pgmFileList);
		System.out.println("Finished reading all files");
		
		DoubleMatrix mu = calcMean(pgmFileList);
		DoubleMatrix X = calcMeanfree(pgmFileList, mu);
		DoubleMatrix Xt = X.transpose();
		DoubleMatrix D = Xt.mmul(X); //Not optimized version: C = X.mmul(Xt)
		System.out.println("Finished calculating D = Xt.mmul(X)");
		
		DoubleMatrix eVectorsD = calcEigenvectors(D);
		
		//For every eigenvektor of D, calculate the real eigenvector for C 
		DoubleMatrix eVectorsC = calcEigenvectorsForC(X, eVectorsD);
		System.out.println("Finished calculating eigenvectors from C=X.mmul(Xt)");
		

		DoubleMatrix eVectorsCT = eVectorsC.transpose();
		DoubleMatrix yToTest = eVectorsCT.mmul(toTestPgm.getMatrix());
		for (PGM pgm : pgmFileList) {
			pgm.distanceToTest = eVectorsCT.mmul(pgm.getMatrix()).distance2(yToTest);
		}

		Collections.sort(pgmFileList);

		
		//add files to the result
		addFileResult(0, new File(pgmFileList.get(0).getPath()));
		addFileResult(1, new File(pgmFileList.get(1).getPath()));
		addFileResult(2, new File(pgmFileList.get(2).getPath()));
		
		//add the corresponding probs to the result
		addDistResult(0, pgmFileList.get(0).distanceToTest);
		addDistResult(1, pgmFileList.get(1).distanceToTest);
		addDistResult(2, pgmFileList.get(2).distanceToTest);
		
		return;
	}

	private DoubleMatrix calcEigenvectorsForC(DoubleMatrix X, DoubleMatrix eVectorsD) {
		DoubleMatrix fi;
		DoubleMatrix eVectorsC = X.mmul(eVectorsD.getColumn(0));
		for (int i = 1; i < eVectorsD.columns; i++) {
			fi = X.mmul(eVectorsD.getColumn(i));
			eVectorsC = DoubleMatrix.concatHorizontally(eVectorsC, fi);
		}
		return eVectorsC;
	}

	private DoubleMatrix calcEigenvectors(DoubleMatrix D) {
		ComplexDoubleMatrix[] complexE = Eigen.eigenvectors(D);

		DoubleMatrix eVectorsD = DoubleMatrix.zeros(complexE[0].getColumns(), complexE[0].getRows());
		for (int i = 0; i < complexE[0].getColumns(); i++) {
			for (int j = 0; j < complexE[0].getRows(); j++) {
				eVectorsD.put(i, j, complexE[0].get(i, j).real());
			}
		}
		return eVectorsD;
	}

	private DoubleMatrix calcMeanfree(List<PGM> pgmFileList, DoubleMatrix mu) {
		DoubleMatrix X = pgmFileList.get(0).getMatrix().sub(mu);
		for (PGM pgm : pgmFileList.subList(1, pgmFileList.size())) {
		//vertically = New matrix will be x atop y. lol wtf
			X = DoubleMatrix.concatHorizontally(X, pgm.getMatrix().sub(mu));
		}
		return X;
	}

	private DoubleMatrix calcMean(List<PGM> pgmFileList) {
		DoubleMatrix mu = new DoubleMatrix(10304); //since all pictures have the same size
		for (PGM pgm : pgmFileList) {
			mu = mu.add(pgm.getMatrix());
		}
		mu = mu.divi(pgmFileList.size());
		return mu;
	}

	private void readAllFiles(InputStream in, List<PGM> pgmFileList) {
		for ( File file : this.set ) {
			try {
				in = new FileInputStream(file.getAbsolutePath());
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}
			pgmFileList.add(new PGM(in, file.getAbsolutePath()));
		}
	}

	private PGM readToTestFile(File toTest, InputStream inTest) {
		try {
			inTest = new FileInputStream(toTest.getAbsolutePath());
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		PGM toTestPgm = new PGM(inTest, toTest.getAbsolutePath());
		return toTestPgm;
	}

}

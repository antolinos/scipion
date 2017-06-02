# **************************************************************************
# *
# * Authors:  Ruben Sanchez (rsanchez@cnb.csic.es), April 2017
# *
# * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# * 02111-1307  USA
# *
# *  All comments concerning this program package may be sent to the
# *  e-mail address 'scipion@cnb.csic.es'
# *
# **************************************************************************


import os

from pyworkflow.utils.path import copyFile, cleanPath 
import pyworkflow.protocol.params as params
from pyworkflow.em.protocol import ProtProcessParticles
import pyworkflow.em.metadata as md
from pyworkflow.em.packages.xmipp3.convert import writeSetOfParticles, setXmippAttributes


class XmippProtScreenDeepLearning1(ProtProcessParticles):
    """ Protocol for screening particles using deep learning. """
    _label = 'screen deep learning 1'
    
    #--------------------------- DEFINE param functions --------------------------------------------
    def _defineParams(self, form):
        form.addSection(label='Input')
        form.addParam('inPosSetOfParticles', params.PointerParam, label="Consensus particles (mostly true particles)", 
                      pointerClass='SetOfParticles',
                      help='Select the intersection set of particles (mostly true particles).')  
        form.addParam('inNegSetOfParticles', params.PointerParam, label="Set of negative particles", 
                      pointerClass='SetOfParticles',
                      help='Select the set of non-particles.')

        form.addParam('testPosSetOfParticles', params.PointerParam, label="Set of positive test particles", 
                      pointerClass='SetOfParticles',
                      help='Select the set of ground true positive particles.')
        form.addParam('testNegSetOfParticles', params.PointerParam, label="Set of negative test particles", 
                      pointerClass='SetOfParticles',
                      help='Select the set of ground false positive particles.')
        
        form.addParam('Nepochs', params.FloatParam, label="Number of epochs", default=4.0, expertLevel=params.LEVEL_ADVANCED,
                      help='Number of epochs for neural network training')  
        form.addParam('learningRate', params.FloatParam, label="Learning rate", default=1e-4, expertLevel=params.LEVEL_ADVANCED,
                      help='Learning rate for neural network training')

        if 'CUDA' in os.environ and not os.environ['CUDA']=="False":
            form.addParallelSection(threads=0, mpi=0)           
        else:
            form.addParallelSection(threads=8, mpi=0)
    
    #--------------------------- INSERT steps functions --------------------------------------------
    def _insertAllSteps(self):
        self._insertFunctionStep('convertInputStep', self.inPosSetOfParticles.get(), self.inNegSetOfParticles.get(), 
                                 self.testPosSetOfParticles.get(), self.testNegSetOfParticles.get()) 
        self._insertFunctionStep('train',self.inPosSetOfParticles.get(), self.inNegSetOfParticles.get(), self.testPosSetOfParticles.get(),
                                 self.testNegSetOfParticles.get(), self.learningRate.get()) 
        self._insertFunctionStep('predict',self.testPosSetOfParticles.get(),self.testNegSetOfParticles.get()) 
        self._insertFunctionStep('createOutputStep')
        
    #--------------------------- STEPS functions --------------------------------------------   
    def convertInputStep(self, inPosSetOfParticles, inNegSetOfParticles, testPosSetOfParticles,testNegSetOfParticles):
        writeSetOfParticles(inPosSetOfParticles, self._getExtraPath("inputTrueParticlesSet.xmd"))
        writeSetOfParticles(inNegSetOfParticles, self._getExtraPath("inputFalseParticlesSet.xmd"))
        writeSetOfParticles(testPosSetOfParticles, self._getExtraPath("testTrueParticlesSet.xmd"))
        writeSetOfParticles(testNegSetOfParticles, self._getExtraPath("testFalseParticlesSet.xmd"))
        
              
    def train(self, inPosSetOfParticles, inNegSetOfParticles, testPosSetOfParticles, testNegSetOfParticles, learningRate):
        '''
        inPosSetOfParticles, inNegSetOfParticles, testPosSetOfParticles, testNegSetOfParticles: SetOfParticles
        learningRate: float
        '''
      
        from pyworkflow.em.packages.xmipp3.deepLearning1 import  DeepTFSupervised, DataManager
        numberOfThreads= None if ('CUDA' in os.environ and not os.environ['CUDA']=="False") \
                              else self.numberOfThreads.get()
        
        trainDataManager= DataManager(posImagesXMDFname= self._getExtraPath("inputTrueParticlesSet.xmd"),
                                       posImagesSetOfParticles= inPosSetOfParticles,
                                       negImagesXMDFname= self._getExtraPath("inputFalseParticlesSet.xmd"),
                                       negImagesSetOfParticles= inNegSetOfParticles)
        testDataManager= DataManager(posImagesXMDFname=  self._getExtraPath("testTrueParticlesSet.xmd"),
                                      posImagesSetOfParticles= testPosSetOfParticles,
                                      negImagesXMDFname= self._getExtraPath("testFalseParticlesSet.xmd"),
                                      negImagesSetOfParticles= testNegSetOfParticles)
 
        numberOfBatches = trainDataManager.getNBatches(self.Nepochs.get())

        nnet = DeepTFSupervised(rootPath=self._getExtraPath("nnetData"), learningRate=learningRate)        
        nnet.createNet( *trainDataManager.shape)
        nnet.startSessionAndInitialize(numberOfThreads)
        
        nnet.trainNet(numberOfBatches, trainDataManager, testDataManager)
        nnet.close(saveModel= True)
        
####        self.predict( testPosSetOfParticles, testNegSetOfParticles)
####        raise ValueError("Debug mode")
        del nnet
        
    def predict(self, testPosSetOfParticles, testNegSetOfParticles):
        from pyworkflow.em.packages.xmipp3.deepLearning1 import  DeepTFSupervised, DataManager
      
        testDataManager= DataManager(posImagesXMDFname=  self._getExtraPath("testTrueParticlesSet.xmd"), 
                              posImagesSetOfParticles= testPosSetOfParticles,
                              negImagesXMDFname= self._getExtraPath("testFalseParticlesSet.xmd"),
                              negImagesSetOfParticles= testNegSetOfParticles)

        nnet = DeepTFSupervised(rootPath=self._getExtraPath("nnetData"))
        numberOfThreads= None if ('CUDA' in os.environ and not os.environ['CUDA']=="False")\
                              else self.numberOfThreads.get()
        nnet.createNet( *testDataManager.shape)
        nnet.startSessionAndInitialize(numberOfThreads)
        y_pred , labels, typeAndIdList = nnet.predictNet(testDataManager)
        
        metadataPos, metadataNeg= testDataManager.getMetadata()            

        for score, label, (mdIsPosType, mdId) in zip(y_pred , labels, typeAndIdList):
          if mdIsPosType==True:
             metadataPos.setValue(md.MDL_ZSCORE_DEEPLEARNING1, float(score), mdId)
          else:
             metadataNeg.setValue(md.MDL_ZSCORE_DEEPLEARNING1, float(score), mdId)
            
        metadataPos.write(self._getPath("particles.xmd"))
        metadataNeg.write(self._getPath("particlesNegative.xmd"))

        
    def createOutputStep(self):
        imgSet = self.testPosSetOfParticles.get()
        partSet = self._createSetOfParticles()
        partSet.copyInfo(imgSet)
        partSet.copyItems(imgSet,
                            updateItemCallback=self._updateParticle,
                            itemDataIterator=md.iterRows(self._getPath("particles.xmd"), sortByLabel=md.MDL_ITEM_ID))
        self._defineOutputs(outputParticles=partSet)
        self._defineSourceRelation(imgSet, partSet)

    
    #--------------------------- INFO functions --------------------------------------------
    def _summary(self):
        summary = []
        return summary
    
    def _methods(self):
        pass
    
    #--------------------------- UTILS functions --------------------------------------------
    def _updateParticle(self, item, row):
        setXmippAttributes(item, row, md.MDL_ZSCORE_DEEPLEARNING1)
        if row.getValue(md.MDL_ENABLED) <= 0:
            item._appendItem = False
        else:
            item._appendItem = True

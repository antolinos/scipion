import os, sys
import numpy as np
import tensorflow as tf
from tensorflow.python.client import device_lib
import tflearn
'''
Inception-like deep net

'''
DROPOUT_KEEP_PROB=0.5

EPSILON=1e-8 # For adam optimizer

DTYPE = tf.float32

def parametric_relu(_x, name="prelu"):
  with tf.variable_scope('prelu'+str(name) ) as scope:
    alphas = tf.get_variable('alpha', _x.get_shape()[-1],
                         initializer=tf.constant_initializer(0.0),
                          dtype=tf.float32)
    pos = tf.nn.relu(_x)
    neg = alphas * (_x - abs(_x)) * 0.5

  return pos + neg
  
relu_function= tf.nn.relu
##relu_function= parametric_relu
    
def _weight_variable(name, shape):
  return tf.get_variable(name, shape, DTYPE, initializer=tf.contrib.layers.xavier_initializer() )

def _bias_variable(name, shape):
  return tf.get_variable(name, shape, DTYPE, tf.constant_initializer(0.0, dtype=DTYPE))


  
def createLastLayer( prev_layer_flat, n_labels):
  with tf.variable_scope('last_softmax_layer' ) as scope:
    dim = np.prod(prev_layer_flat.get_shape().as_list()[1:])
    weights = _weight_variable('weights', [dim, n_labels])
    biases = _bias_variable('biases', [n_labels])
    logits = tf.matmul(prev_layer_flat, weights) + biases
    y_pred = tf.nn.softmax(logits)
  return logits, y_pred
   
def conv2d_s1(prev_layer, W):
  with tf.variable_scope('conv2d') as scope:
    return tf.nn.conv2d(prev_layer,W,strides=[1,1,1,1],padding='SAME')
 
def max_pool_3x3_s1(prev_layer):
  with tf.variable_scope('max_pool3x3' ) as scope:
    return tf.nn.max_pool(prev_layer,ksize=[1,3,3,1], strides=[1,1,1,1],padding='SAME')

def avg_pool_5x5_s1(prev_layer):
  with tf.variable_scope('avg_pool5x5' ) as scope:
    return tf.nn.avg_pool(prev_layer,ksize=[1,5,5,1], strides=[1,2,2,1],padding='SAME')


def createOneConvLayer(layerNum, prev_layer_out, outChanNum, kernerSize=3, poolingStep=1):

  in_filters= prev_layer_out.get_shape().as_list()[-1]
  with tf.variable_scope('conv'+str(layerNum) ) as scope:
    kernel = _weight_variable('weights', [kernerSize, kernerSize, in_filters, outChanNum])
    conv = tf.nn.conv2d(prev_layer_out, kernel, [1, 1, 1, 1], padding='SAME')
    biases = _bias_variable('biases', [outChanNum])
    bias = tf.nn.bias_add(conv, biases)
    conv_out = relu_function(bias, name="relu")
    if poolingStep>1:
      with tf.variable_scope('maxpool') as scope:
        conv_out = tf.nn.max_pool( conv_out, ksize=[1, kernerSize, kernerSize, 1], 
                                     strides=[1, poolingStep, poolingStep, 1], padding='SAME')
    return conv_out, kernel
        
def createInceptionModule(layerNum, prev_layer, outChanNum, poolingStep=-1):
  with tf.variable_scope('inceptionModule_'+str(layerNum) ) as scope:
    prev_layer_shapes= prev_layer.get_shape().as_list()
    
    with tf.variable_scope('conv_1x1' ) as scope:
      W_conv_1x1_1= _weight_variable("conv_1x1_1weight", shape= [1,1, prev_layer_shapes[-1], outChanNum] )
      b_conv_1x1_1= _bias_variable("conv_1x1_1bias", shape= [outChanNum] )
      conv_1x1_1 = conv2d_s1(prev_layer,W_conv_1x1_1) + b_conv_1x1_1
      
    with tf.variable_scope('conv_3x3' ) as scope:
      with tf.variable_scope('conv_1x1' ) as scope:
        W_conv_1x1_2= _weight_variable("conv_1x1_2weight", shape= [1,1, prev_layer_shapes[-1], outChanNum//2] )
        b_conv_1x1_2= _bias_variable("conv_1x1_2bias", shape= [outChanNum//2] )
        conv_1x1_2 = relu_function(conv2d_s1(prev_layer,W_conv_1x1_2) + b_conv_1x1_2)
        
      with tf.variable_scope('conv_3x3' ) as scope:
        W_conv_3x3= _weight_variable("conv_3x3_weight", shape= [3,3, outChanNum//2, outChanNum] )
        b_conv_3x3= _bias_variable("conv_3x3_bias", shape= [outChanNum] )  
        conv_3x3 = conv2d_s1(conv_1x1_2, W_conv_3x3)+ b_conv_3x3
      
    with tf.variable_scope('conv_5x5' ) as scope:
      with tf.variable_scope('conv_1x1' ) as scope:    
        W_conv_1x1_3= _weight_variable("conv_1x1_3weight", shape= [1,1, prev_layer_shapes[-1], outChanNum//2] )
        b_conv_1x1_3= _bias_variable("conv_1x1_3bias", shape= [outChanNum//2] ) 
        conv_1x1_3 = relu_function(conv2d_s1(prev_layer, W_conv_1x1_3)+b_conv_1x1_3)
        
      with tf.variable_scope('conv_5x5' ) as scope:      
        W_conv_5x5= _weight_variable("conv_5x5_weight", shape= [5,5, outChanNum//2, outChanNum] )
        b_conv_5x5= _bias_variable("conv_5x5_bias", shape= [outChanNum] )  
        conv_5x5 = conv2d_s1(conv_1x1_3, W_conv_5x5)+ b_conv_5x5

    with tf.variable_scope('max_pool_3x3' ) as scope:
      with tf.variable_scope('max_pool_3x3' ) as scope:    
        maxpool1 = max_pool_3x3_s1(prev_layer)
      with tf.variable_scope('conv_1x1' ) as scope:          
        W_conv_1x1_4= _weight_variable("conv_1x1_4weight", shape= [1,1, prev_layer_shapes[-1], outChanNum] )
        b_conv_1x1_4= _bias_variable("conv_1x1_4bias", shape= [outChanNum] )
        conv_1x1_4 = conv2d_s1(maxpool1, W_conv_1x1_4)+b_conv_1x1_4
      
    conv_out= relu_function(tf.concat( values=[conv_1x1_1, conv_3x3, conv_5x5, conv_1x1_4], axis=3 ) )
  if poolingStep>1:
    with tf.variable_scope('maxpool_Incep_'+str(layerNum) ) as scope:
      conv_out = tf.nn.max_pool( conv_out, ksize=[1, 3, 3, 1], 
                                   strides=[1, poolingStep, poolingStep, 1], padding='SAME')
  return conv_out

def main_network(x, labels, num_labels, learningRate, globalStep):
  '''
    4D-tensor x,  [imageNumber, height, width, nChanns]
    2D-tensor labels, [imageNumber, 2]
    float learningRate
    tf.Variable globalStep
  '''

  img_aug= tflearn.data_augmentation.ImageAugmentation()
  img_aug.add_random_flip_leftright()
  img_aug.add_random_flip_updown()

  prev_layer = tflearn.layers.core.input_data(placeholder=x, data_augmentation=img_aug)

  NUM_FILTERS=[32, 48, 48,  64, 80, 96]
  KERNEL_SIZE=[5,   1,  3,  -1, -1, -1]
  MAX_POOL=   [2,   1,  2,  -1,  2, -1]
  
  for i in range(3):
    prev_layer = tflearn.conv_2d(prev_layer, NUM_FILTERS[i], KERNEL_SIZE[i],activation="relu", regularizer= None)
    if MAX_POOL[i]>1:
      prev_layer = tflearn.layers.conv.max_pool_2d(prev_layer, kernel_size=3,  strides= MAX_POOL[i])
      prev_layer = tflearn.layers.normalization.local_response_normalization(prev_layer)
    
  prev_layer= createInceptionModule(0, prev_layer, NUM_FILTERS[-3], MAX_POOL[-3] )  
##  prev_layer= tflearn.layers.normalization.batch_normalization (prev_layer)  
  prev_layer= createInceptionModule(1, prev_layer, NUM_FILTERS[-2], MAX_POOL[-2] )
  prev_layer= createInceptionModule(2, prev_layer, NUM_FILTERS[-1], MAX_POOL[-1] )
        
  prev_layer_flat = tflearn.global_avg_pool(prev_layer)
  
  prev_layer_flat= tflearn.fully_connected(prev_layer_flat, 1024, activation='relu', regularizer='L2', weight_decay=1e-4)
  prev_layer_flat= tflearn.layers.core.dropout(prev_layer_flat, keep_prob=DROPOUT_KEEP_PROB) 
  logits = tflearn.fully_connected( prev_layer_flat, num_labels)
  y_pred = tf.nn.softmax(logits)
  
  optimizer = tf.train.AdamOptimizer(learning_rate= learningRate, epsilon= EPSILON)


  reg_losses = tf.get_collection(tf.GraphKeys.REGULARIZATION_LOSSES)
  cross_entropy= tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits= logits, labels= labels) + sum(reg_losses) )
  
  optimizer= optimizer.minimize(cross_entropy, global_step= globalStep)
  
  with tf.name_scope('PERFORMANCE'):
    with tf.name_scope('correct_prediction'):
      correct_prediction = tf.equal(tf.argmax(y_pred, 1), tf.argmax(labels, 1))
    with tf.name_scope('accuracy'):
      accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))
      
    tf.summary.scalar( 'cross_entropy', cross_entropy )
    tf.summary.scalar( 'accuracy', accuracy )
#    tf.summary.image("1_images", x, max_outputs= 3)
#    tf.summary.image("3_kernel_0",  tf.transpose (w_0, [3, 0, 1, 2]), max_outputs= NUM_FILTERS[0]) 
#    for kerNum in range( NUM_FILTERS[0]):
#      tf.summary.image("2_activations_0", tf.expand_dims(prev_layer0[:,:,:,kerNum], 3), max_outputs= 3)
#    for kerNum in range( NUM_FILTERS[1]):
#      tf.summary.image("3_activations_0", tf.expand_dims(prev_layer1[:,:,:,kerNum], 3), max_outputs= 3)            
#    for kerNum in range( NUM_FILTERS[2]):
#      tf.summary.image("3_activations_0", tf.expand_dims(prev_layer2[:,:,:,kerNum], 3), max_outputs= 3)           
    mergedSummaries= tf.summary.merge_all()

  return y_pred, mergedSummaries, optimizer, cross_entropy, accuracy


pipeline {
  agent none
  stages {
    stage('Prepare code') {
      steps {
        sh '''git submodule sync
git submodule update --init'''
        sh '''cd $WORKSPACE/GUI/res
unzip oxygen.zip'''
      }
    }
    stage('Compile uC Bootloader') {
      parallel {
        stage('Compile uC Bootloader') {
          agent {
            docker {
              image 'fpoussin/jenkins:ubuntu-18.04-arm'
            }

          }
          steps {
            sh '''cd $WORKSPACE/code/bootloader
make clean
make -j $(nproc)'''
          }
        }
        stage('Compile uC Application') {
          agent {
            docker {
              image 'fpoussin/jenkins:ubuntu-18.04-arm'
            }

          }
          steps {
            sh '''cd $WORKSPACE/code/app/dsp_lib
make clean
make -j $(nproc)
cd ..
make clean
make -j $(nproc)
'''
          }
        }
      }
    }
    stage('Compile GUI') {
      agent {
        docker {
          image 'fpoussin/jenkins:ubuntu-18.04-qt5'
        }

      }
      steps {
        sh '''cd $WORKSPACE/GUI
qmake
make -j $(nproc)
'''
      }
    }
  }
}
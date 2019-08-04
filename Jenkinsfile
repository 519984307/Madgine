

 def axisList = [
    [//toolchains
		[
			name : "clang-linux",
			dockerImage : 'schuetzo/linux-test-env:latest'
		],
		[
			name : "clang-android",
			dockerImage : 'schuetzo/linux-test-env:latest'
		],
		[
			name : "emscripten",
			dockerImage : 'node:latest'
		]
	],           
    [//configurations
		[
			name : "Debug"
		],
		[
			name : "RelWithDebInfo"
		]
	]
]   

def staticConfigs = ["OpenGL"]

def tasks = [:]
def comboBuilder
def comboEntry = []
def comboNames = []


def task = {
    // builds and returns the task for each combination

    /* Map the entries back to a more readable format
       the index will correspond to the position of this axis in axisList[] */
    def toolchain = it[0]
    def configuration = it[1]
    //def staticConfig = it[2]
	
    def name = toolchain.name + '-' + configuration.name  

	//if (staticConfig?.trim())
	//	staticConfig = "test/${staticConfig}_${toolchain}.cfg"

    return {
        // This is where the important work happens for each combination
	    stage ("${name}") {
            stage("cmake") {
			    sh """
				if ${params.fullBuild}; then
					if [ -d "${name}" ]; then 
						rm -Rf ${name};
					fi
				fi
				mkdir -p ${name}
				cd ${name}
			    cmake .. \
		        -DCMAKE_BUILD_TYPE=${configuration.name} \
		        -DCMAKE_TOOLCHAIN_FILE=~/toolchains/${toolchain.name}.cmake
			    """
		                //    -DSTATIC_BUILD=${staticConfig}
            }
            stage("build") {				
				sh """
				cd ${name}
				make -j4 all 
				"""				
            }
		    docker.image(toolchain.dockerImage).inside {
				stage("Test") {
					sh """
					cd ${name}
					ctest
					"""
				}
            }           
        }
    }
}

/*
    This is where the magic happens
    recursively work through the axisList and build all combinations
*/
comboBuilder = { def axes, int level ->
    for ( entry in axes[0] ) {
        comboEntry[level] = entry
		comboNames[level] = entry.name
        if (axes.size() > 1 ) {
            comboBuilder(axes.drop(1), level + 1)
        }
        else {
            tasks[comboNames.join("-")] = task(comboEntry.collect())
        }
    }
}

comboBuilder(axisList, 0)    

pipeline {
    agent any

	parameters {
        booleanParam(defaultValue: false, description: '', name: 'fullBuild')
    }

	options{
		ansiColor('xterm')
		disableConcurrentBuilds()
	}

    stages {
		stage("checkout") {
			steps{
        		checkout scm
				sh """
				git submodule update --init --recursive
				"""
			}
	    }
        stage ("Multiconfiguration Parallel Tasks") {
	        steps {
			    script {
				    parallel tasks
			    }
	        }
        }
    }

	post {
        always {
            junit '**/*.xml'
		
	    recordIssues enabledForFailure: true, tools: [clang()]
        }
    }
}



 def axisList = [
    ["clang-linux",],           //toolchain
    ["Release","Debug"],      //configuration
    ["",]                   //static configuration file
]   

def tasks = [:]
def comboBuilder
def comboEntry = []


def task = {
    // builds and returns the task for each combination

    /* Map the entries back to a more readable format
       the index will correspond to the position of this axis in axisList[] */
    def toolchain = it[0]
    def configuration = it[1]
    def staticConfig = it[2]
	
    def name = "${it.join('-')}"    

	if (staticConfig?.trim())
		staticConfig = "../${staticConfig}"

    return {
        // This is where the important work happens for each combination
        node {
		    stage("checkout ${name}") {
           		checkout scm
                sh """
			    git submodule update --init --recursive
                """
	    	}
            stage("cmake ${name}") {
			    sh """
                mkdir -p ${name}
				cd ${name}
			    cmake .. \
		        -DCMAKE_BUILD_TYPE=${configuration} \
		        -DCMAKE_TOOLCHAIN_FILE=~/toolchains/${toolchain}.cmake \
                -DSTATIC_BUILD=${staticConfig} \
				-DWorkspace=workspace
			    """
            }
            stage("build ${name}") {
                sh """
                cd ${name}
                make
                """
            }
			stage("Test ${name}") {
				sh """
				cd ${name}
				rm -rf memchecks
				mkdir -p memchecks
				ctest -T memcheck
				"""
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
        if (axes.size() > 1 ) {
            comboBuilder(axes.drop(1), level + 1)
        }
        else {
            tasks[comboEntry.join("-")] = task(comboEntry.collect())
        }
    }
}

comboBuilder(axisList, 0)    

pipeline {
    agent any

    stages {
        stage ("Multiconfiguration Parallel Tasks") {
	        steps {
			    script {
				    parallel tasks
			    }
	        }
        }
    }
}

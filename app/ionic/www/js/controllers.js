angular.module('controllers', [])
.controller('CameraCtrl', function($scope, $http, cameraSettings) {

	//set up all the sliders with the data grabbed from the server
	for(var i = 0; i < cameraSettings.length; i++){
		console.log("cameraSettings[" + i +"] = "+ cameraSettings[i].name);
		if(cameraSettings[i].name == "CAMERA_OFFSET"){
			$scope.CAMERA_OFFSET = {name: cameraSettings[i].name, value:cameraSettings[i].value};
		}
		if(cameraSettings[i].name == "CAMERAS"){
			$scope.CAMERAS = {name: cameraSettings[i].name, value:cameraSettings[i].value};
		}
	}
    $scope.imagePath = '/out.jpg' + "?dummy=" + Math.floor(Math.random() * 100); // default to image 1
    $scope.HIGHLIGHT = false;

    // Functions
    $scope.setHighlighted = function() {
        if ($scope.HIGHLIGHT == true) {
            $scope.HIGHLIGHT = false;
        } else {
            $scope.HIGHLIGHT = true;
        }
        $scope.refreshImage();
        $scope.updateCameraSettings();
    }
	$scope.updateCameraSettings = function(){   
		var data = {};
		data.CAMERA_OFFSET = $scope.CAMERA_OFFSET.value;
        data.CAMERAS = $scope.CAMERAS.value;
        data.HIGHLIGHT = $scope.HIGHLIGHT;
		console.log("data="+JSON.stringify(data));
		$http({
			method: 'POST',
			url: "http://" + location.host + "/update",
			data: data,
			headers: {'Content-Type': 'application/x-www-form-urlencoded'}
		});
        $scope.refreshImage();
	}
	$scope.refreshImage = function(){
		$scope.imagePath = "out.jpg" + "?dummy=" + Math.floor(Math.random() * 100); // default to image 1
	}
	$scope.resetToFactoryDefaults = function(){
        // reset sliders locally
		$scope.CAMERAS.value = 2;
		$scope.CAMERA_OFFSET.value = 0;
		// reset sliders on server
		var data = {};
		data.CAMERAS = 2;
		data.CAMERA_OFFSET = 0;
		console.log("data="+JSON.stringify(data));
        $scope.refreshImage();
		$http({
		    method: 'POST',
            url: "http://" + location.host + "/update",
            data: data,
            headers: {'Content-Type': 'application/x-www-form-urlencoded'}
	    });
	}
})

// Basic Settings
.controller('DashCtrl', function($scope, $http, dashSettings) {
	
    // set up all the sliders with the data grabbed from the server
	for(var i = 0; i < dashSettings.length; i++){
		console.log("dashSettings[" + i +"] = "+ dashSettings[i].name);
		if(dashSettings[i].name == "SENSITIVITY"){
			$scope.SENSITIVITY = {name: dashSettings[i].name, value:dashSettings[i].value};
		}
		if(dashSettings[i].name == "AGGRESSIVENESS"){
			$scope.AGGRESSIVENESS = {name: dashSettings[i].name, value:dashSettings[i].value};
		}
		if(dashSettings[i].name == "MIN_VOLTAGE"){
			$scope.MIN_VOLTAGE = {name: dashSettings[i].name, value:dashSettings[i].value};
		}
		if(dashSettings[i].name == "MAX_VOLTAGE"){
			$scope.MAX_VOLTAGE = {name: dashSettings[i].name, value:dashSettings[i].value};
		}
		if(dashSettings[i].name == "SUPPLY_VOLTAGE"){
			$scope.SUPPLY_VOLTAGE = {name: dashSettings[i].name, value:dashSettings[i].value};
		}
	}
    $scope.imagePath = "out.jpg" + "?dummy=" + Math.floor(Math.random() * 100); // default to image 1
    $scope.PWM_INVERTED = false;
    
    // Functions
    $scope.setInverted = function() {
        if ($scope.PWM_INVERTED == true) {
            $scope.PWM_INVERTED = false;
        } else {
            $scope.PWM_INVERTED = true;
        }
    }
	$scope.updateDashSettings = function(){   
		var data = {};
		data.SENSITIVITY = $scope.SENSITIVITY.value;
        data.AGGRESSIVENESS = $scope.AGGRESSIVENESS.value;
        data.MIN_VOLTAGE = $scope.MIN_VOLTAGE.value;
        data.MAX_VOLTAGE = $scope.MAX_VOLTAGE.value;
        data.SUPPLY_VOLTAGE = $scope.SUPPLY_VOLTAGE.value;
        data.PWM_INVERTED = $scope.PWM_INVERTED;
		$http({
			method: 'POST',
			url: "http://" + location.host + "/update",
			data: data,
			headers: {'Content-Type': 'application/x-www-form-urlencoded'}
		});
        $scope.refreshImage();
	}
	$scope.runCalibration = function(){   
		var data = {};
		$scope.updateDashSettings();
		$http({
			method: 'POST',
			url: "http://" + location.host + "/calibrate",
			data: data,
			headers: {'Content-Type': 'application/x-www-form-urlencoded'}
		});
	}
	$scope.resetToFactoryDefaults = function(){
		// reset sliders locally
		$scope.SENSITIVITY.value = 1;
		$scope.AGGRESSIVENESS.value = 1;
        $scope.MIN_VOLTAGE.value = 1250;
        $scope.MAX_VOLTAGE.value = 3750;
        $scope.SUPPLY_VOLTAGE.value = 5000;
		// reset sliders on server
		var data = {};
		data.SENSITIVITY = 1;
		data.AGGRESSIVENESS = 1;
        data.MIN_VOLTAGE = 1250;
        data.MAX_VOLTAGE = 3750;
        data.SUPPLY_VOLTAGE = 5000;
        $scope.refreshImage();
		$http({
			method: 'POST',
			url: "http://" + location.host + "/update",
			data: data,
			headers: {'Content-Type': 'application/x-www-form-urlencoded'}
		});
	}
	$scope.refreshImage = function(){
        $scope.imagePath = "out.jpg" + "?dummy=" + Math.floor(Math.random() * 100); // default to image 1
	}
})

// Advanced Settings
.controller('advancedCtrl', function($scope, $http, advancedSettings) {
	// Set up all the sliders with the data grabbed from the server
	for(var i = 0; i < advancedSettings.length; i++){
		console.log("advancedSettings[i].name"+advancedSettings[i].name);
		if(advancedSettings[i].name == "P_COEF"){
			$scope.P_COEF = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "I_COEF"){
			$scope.I_COEF = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "D_COEF"){
			$scope.D_COEF = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "HUE_MIN"){
			$scope.HUE_MIN = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "HUE_MAX"){
			$scope.HUE_MAX = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "SAT_MIN"){
			$scope.SAT_MIN = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "SAT_MAX"){
			$scope.SAT_MAX = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "VAL_MIN"){
			$scope.VAL_MIN = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "VAL_MAX"){
			$scope.VAL_MAX = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "N_SAMPLES"){
			$scope.N_SAMPLES = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
		if(advancedSettings[i].name == "THRESHOLD_PERCENTILE"){
			$scope.THRESHOLD_PERCENTILE = {name: advancedSettings[i].name, value:advancedSettings[i].value};
		}
	}

	$scope.updateAdvancedSettings = function(){   
        // reset sliders locally		
        var data = {};
		data.P_COEF = $scope.P_COEF.value;
		data.I_COEF = $scope.I_COEF.value;
		data.D_COEF = $scope.D_COEF.value;
		data.HUE_MIN = $scope.HUE_MIN.value;
		data.HUE_MAX = $scope.HUE_MAX.value;
		data.SAT_MIN = $scope.SAT_MIN.value;
		data.SAT_MAX = $scope.SAT_MAX.value;
		data.VAL_MIN = $scope.VAL_MIN.value;
		data.VAL_MAX = $scope.VAL_MAX.value;
		data.N_SAMPLES = $scope.N_SAMPLES.value;
		data.THRESHOLD_PERCENTILE = $scope.THRESHOLD_PERCENTILE.value;
		console.log("data="+JSON.stringify(data));
        $scope.refreshImage();
		$http({
			method: 'POST',
			url: "http://" + location.host + "/update",
			data: data,
			headers: {'Content-Type': 'application/x-www-form-urlencoded'}
		});
	}

	$scope.resetToFactoryDefaults = function(){

		// reset sliders locally
		$scope.P_COEF.value = 1;
		$scope.I_COEF.value = 4;
		$scope.D_COEF.value = 0;
		$scope.HUE_MIN.value = 45;
		$scope.HUE_MAX.value = 105;
		$scope.SAT_MIN.value = 128;
		$scope.SAT_MAX.value = 255;
		$scope.VAL_MIN.value = 64;
		$scope.VAL_MAX.value = 250;
        $scope.N_SAMPLES.value = 30;
        $scope.THRESHOLD_PERCENTILE.value = 95;

		//reset sliders on server
		var data = {};
		data.HUE_MIN = 45;
		data.HUE_MAX = 105;
		data.SAT_MIN = 128;
		data.SAT_MAX = 255;
		data.VAL_MIN = 64;
		data.VAL_MAX = 250;
		data.P_COEF = 1;
		data.I_COEF = 4;
		data.D_COEF = 0;
        data.N_SAMPLES = 30;
        data.THRESHOLD_PERCENTILE = 95;
		$http({
			method: 'POST',
			url: "http://" + location.host + "/update",
			data: data,
			headers: {'Content-Type': 'application/x-www-form-urlencoded'}
		});
	}

})

.controller('AboutCtrl', function($scope) {
    $scope.saveLog = function () {
        window.open("/logs/log.txt");
    }
});

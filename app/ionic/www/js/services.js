angular.module('services', [])

// Adavanced Settings Service
// change $http to $timeout for debug mode
.factory('advancedSettingsService', ['$http', '$q', function($http, $q) {
	var Service = {};
	
	Service.getAdvancedSettings = function(){
		var defer = $q.defer();

		// DO NOT ERASE
		//This is part that requests the data from the server. Below the commented out code is my mock of the data.
        $http({
			method: 'GET',
			url: "http://" + location.host + "/config",
		}).then(function successCallback(response) {
            console.log(response);
            // this callback will be called asynchronously when the response is available
            defer.resolve([
				{name: "I_COEF", value: response.data["I_COEF"]},
				{name: "P_COEF", value: response.data["P_COEF"]},
				{name: "D_COEF", value: response.data["D_COEF"]},
                {name: "N_SAMPLES", value: response.data["N_SAMPLES"]},
				{name: "HUE_MIN", value: response.data["HUE_MIN"]},
				{name: "HUE_MAX", value: response.data["HUE_MAX"]},
				{name: "SAT_MIN", value: response.data["SAT_MIN"]},
				{name: "SAT_MAX", value: response.data["SAT_MAX"]},
				{name: "VAL_MIN", value: response.data["VAL_MIN"]},
				{name: "VAL_MAX", value: response.data["VAL_MAX"]},
                {name: "THRESHOLD_PERCENTILE", value: response.data["THRESHOLD_PERCENTILE"]}
			]);
        }, function errorCallback(response) {
            // called asynchronously if an error occurs or server returns response with an error status.
            console.log(response);
            defer.resolve([
				{name: "I_COEF", value:1},
				{name: "P_COEF", value:4},
				{name: "D_COEF", value:0},
                {name: "N_SAMPLES", value:30},
				{name: "HUE_MIN", value:45},
				{name: "HUE_MAX", value:105},
				{name: "SAT_MIN", value:128},
				{name: "SAT_MAX", value:255},
				{name: "VAL_MIN", value:64},
				{name: "VAL_MAX", value:250},
                {name: "THRESHOLD_PERCENTILE", value:95}
			]);
        });

        // This is just mocking out the server
        // change $http to $timeout in the function call for debug mode
		/*$timeout(function(){
			defer.resolve([
				{name: "I_COEF", value:1},
				{name: "P_COEF", value:4},
				{name: "D_COEF", value:0},
                {name: "N_SAMPLES", value:30},
				{name: "HUE_MIN", value:45},
				{name: "HUE_MAX", value:105},
				{name: "SAT_MIN", value:128},
				{name: "SAT_MAX", value:255},
				{name: "VAL_MIN", value:64},
				{name: "VAL_MAX", value:250},
			]);
		}, 10);*/
		return defer.promise;
	}
	return Service;
}])

// Camera Settings
.factory('cameraSettingsService', ['$http', '$q', function($http, $q) {
	var Service = {};
	Service.getCameraSettings = function(){
		var defer = $q.defer();
        $http({
			method: 'GET',
			url: "http://" + location.host + "/config",
		}).then(function successCallback(response) {
            console.log(response);
            // this callback will be called asynchronously when the response is available
            defer.resolve([
				{name: "CAMERAS", value: response.data["CAMERAS"]},
				{name: "CAMERA_OFFSET", value: response.data["CAMERA_OFFSET"]}
			]);
        }, function errorCallback(response) {
            // called asynchronously if an error occurs or server returns response with an error status.
            console.log(response);
            defer.resolve([
				{name: "CAMERAS", value: 1},
				{name: "CAMERA_OFFSET", value: 0}
			]);
        });

        // This is just mocking out the server
        // change $http to $timeout in the function call for debug mode
		/*$timeout(function(){
			defer.resolve([
				{name: "CAMERA_OFFSET", value: 0, min:-10, max:10},
                {name: "CAMERAS", value:2}
			]);
		}, 10);*/
		return defer.promise;
	}
	return Service;
}])

// Basic Settings Service
.factory('dashSettingsService', ['$http', '$q', function($http, $q) {
	var Service = {};

	Service.getDashSettings = function(){
		var defer = $q.defer();

		// DO NOT ERASE
		//This is part that requests the data from the server. Below the commented out code is my mock of the data.
		$http({
			method: 'GET',
			url: "http://" + location.host + "/config",
		}).then(function successCallback(response) {
            console.log(response);
            // this callback will be called asynchronously
            // when the response is available
            defer.resolve([
				{name: "SENSITIVITY", value: response.data["SENSITIVITY"]},
				{name: "AGGRESSIVENESS", value: response.data["AGGRESSIVENESS"]},
                {name: "MIN_VOLTAGE", value: response.data["MIN_VOLTAGE"]},
                {name: "MAX_VOLTAGE", value: response.data["MAX_VOLTAGE"]},
                {name: "SUPPLY_VOLTAGE", value: response.data["SUPPLY_VOLTAGE"]}
			]);
        }, function errorCallback(response) {
            // called asynchronously if an error occurs
            // or server returns response with an error status.
            console.log(response);
            defer.resolve([
				{name: "SENSITIVITY", value: 1},
				{name: "AGGRESSIVENESS", value: 1},
                {name: "MIN_VOLTAGE", value: 1250},
                {name: "MAX_VOLTAGE", value: 3750},
                {name: "SUPPLY_VOLTAGE", value: 5000}
			]);
        });

        // This is just mocking out the server
        // change $http to $timeout in the function call for debug mode
		/*$timeout(function(){
			defer.resolve([
				{name: "SENSITIVITY", value: 1, min:1, max:10},
				{name: "AGGRESSIVENESS", value: 1, min:1, max:10}
			]);
		}, 10);*/
		return defer.promise;
	}
	return Service;
}])

var app;
(function (app) {
    function getParameterByName(name, url) {
        if (!url)
            url = window.location.search;
        name = name.replace(/[\[\]]/g, "\\$&");
        var results = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)").exec(url);
        if (!results)
            return null;
        if (!results[2])
            return '';
        return decodeURIComponent(results[2].replace(/\+/g, " "));
    }
    function Fetch() {
        var fileName = "http://192.168.1.2:8080/api";
        console.log("Starting fetch for " + fileName);
        FetchInternal(fileName);
    }
    app.Fetch = Fetch;
    function FetchInternal(fileName) {
        var request = new XMLHttpRequest();
        request.addEventListener("load", function (e) { return TransferComplete(e, fileName); });
        request.addEventListener("error", function (e) { return TransferFailed(e, fileName); });
        request.addEventListener("abort", function (e) { return TransferCanceled(e, fileName); });
        request.open("GET", fileName);
        request.send();
    }
    app.FetchInternal = FetchInternal;
    function TransferComplete(evt, fileName) {
        var result = JSON.parse(evt.target.responseText);
        app.DisplayData(result);
        setTimeout(function () {
            app.FetchInternal(fileName);
        }, 500);
    }
    function TransferFailed(evt, fileName) {
        console.error("An error occurred while transferring the file.", evt);
        setTimeout(function () {
            app.FetchInternal(fileName);
        }, 2500);
    }
    function TransferCanceled(evt, fileName) {
        console.error("The transfer has been canceled by the user.", evt);
    }
})(app || (app = {}));
var app;
(function (app) {
    function DisplayData(data) {
        SetNumber("#velocity", data.vel, 1);
        SetPercentage("#slip", data.slip, {
            Good: { Min: 0, Max: 20 },
            Warning: { Min: 20, Max: 40 },
            Bad: { Min: 40, Max: 100 }
        });
        SetCVT("#cvt_ratio", data.cvt_ratio);
        SetPercentage("#cvt_pct", data.cvt_pct);
        SetNumber("#rpm", data.rpm, 0);
        SetPercentage("#throttle", data.throttle);
        SetPercentage("#engine-loading", data.load, {
            Good: { Min: 0, Max: 80 },
            Warning: { Min: 80, Max: 90 },
            Bad: { Min: 90, Max: 100 }
        });
        SetDegrees("#engine-temperature", data.eng_temp, {
            Good: { Min: 0, Max: 100 },
            Warning: { Min: 100, Max: 140 },
            Bad: { Min: 141, Max: 1000 }
        });
        SetNumber("#oil-pressure", data.oil, 0, {
            Good: { Min: 0, Max: 50 },
            Bad: { Min: 50, Max: 1000 }
        });
        SetPercentage("#suspension", data.susp);
        SetBallast("#ballast-state", data.ballast);
        SetPercentage("#lhs-brake-state", data.lbrake, {
            Good: { Min: 0, Max: 0 },
            Warning: { Min: 0, Max: 100 }
        });
        SetPercentage("#rhs-brake-state", data.rbrake, {
            Good: { Min: 0, Max: 0 },
            Warning: { Min: 0, Max: 100 }
        });
        SetNumber("#engine-hours", data.hours, 0);
        SetBatteryVoltage("#battery-voltage", data.bat);
        SetValue("#rfid-user", data.user);
        SetValue("#differential", data.lock ? "ON" : "OFF", data.lock ? "warning" : "");
        SetNumber("#gear", data.gear, 0);
        SetPercentage("#belt_slip", data.belt_slip);
        SetDegrees("#trans_temp", data.trans_temp);
    }
    app.DisplayData = DisplayData;
    function RangeClassFromRanges(value, ranges) {
        if (!ranges) {
            return "";
        }
        if (ranges.Good && ranges.Good.Min <= value && ranges.Good.Max >= value) {
            return "good";
        }
        if (ranges.Warning && ranges.Warning.Min <= value && ranges.Warning.Max >= value) {
            return "warning";
        }
        if (ranges.Bad && ranges.Bad.Min <= value && ranges.Bad.Max >= value) {
            return "bad";
        }
    }
    function SetPercentage(selector, value, ranges) {
        SetValue(selector, value.toString() + " %", RangeClassFromRanges(value, ranges));
    }
    function SetDegrees(selector, value, ranges) {
        SetValue(selector, value.toFixed(1).toString() + " °C", RangeClassFromRanges(value, ranges));
    }
    function SetBallast(selector, value) {
        var stringValue = "OFF";
        switch (value) {
            case "Forward":
                stringValue = "FWD";
                break;
            case "Backward":
                stringValue = "REV";
                break;
        }
        SetValue(selector, stringValue);
    }
    function SetCVT(selector, value) {
        if (value == -1) {
            SetValue(selector, "-");
        }
        else {
            SetNumber(selector, value);
        }
    }
    function SetBatteryVoltage(selector, value) {
        var rangeClass = RangeClassFromRanges(value, {
            Good: { Min: 11, Max: 12.999999 },
            Warning: { Min: 13, Max: 15 },
            Bad: { Min: 15.000001, Max: 1000 }
        });
        if (value < 11) {
            rangeClass = "warning";
        }
        SetValue(selector, value.toFixed(1).toString(), rangeClass);
    }
    function SetNumber(selector, value, precision, ranges) {
        if (precision === void 0) { precision = 2; }
        SetValue(selector, value.toFixed(precision).toString(), RangeClassFromRanges(value, ranges));
    }
    function SetValue(selector, value, rangeClass) {
        var root = document.querySelector(selector);
        root.classList.remove("good", "warning", "bad");
        if (rangeClass) {
            root.classList.add(rangeClass);
        }
        var element = root.querySelector(".value");
        element.innerHTML = value.toString();
    }
})(app || (app = {}));
var app;
(function (app) {
    window.onload = function () {
        var data = {
            vel: 15.6,
            slip: 10,
            cvt_ratio: 3.4,
            cvt_pct: 26,
            rpm: 3600,
            throttle: 100,
            load: 87,
            eng_temp: 80,
            oil: 40,
            susp: 100,
            ballast: "Forward",
            lbrake: 0,
            rbrake: 75,
            hours: 10.5,
            bat: 12.6,
            user: "guest",
            lock: false,
            gear: 0,
            belt_slip: -5.0,
            trans_temp: 30
        };
        app.DisplayData(data);
        app.Fetch();
    };
})(app || (app = {}));
//# sourceMappingURL=bundle.js.map
var alldata; // Global access to data for debug

var DATA_RESOLUTION = {
  'minute': 1,
  'hour': 2,
  'day': 3
};
var pointThreshold = 3000; // 2000hrs ~ 83 days, 2000 min ~ 33hr
/* Wrapper to get data in time range, determines resolution first,
 * Data resolution is either minutes, hours, or days, depending on range
 * such that number of points below pointThreshold 
 * set_data_fn is the callback passed the data that is returned
 */
function getTempsInRange(date_min, date_max, set_data_fn) {
  console.log("Asked for range", new Date(date_min) + " <--> " + new Date(date_max));
  var diffMs = date_max - date_min; // Delta in milliseconds
  var diffMinutes = diffMs / (1000 * 60); // Delta in minutes
  var diffHours = diffMinutes / 60; // Delta in hour

  console.log((diffHours/24).toFixed(2) + " days,", diffMinutes.toFixed(2) + " minutes,", diffHours.toFixed(2) + " hours in range");

  // Determine resolution
  var resolution;
  if (diffHours > pointThreshold) {
    resolution = DATA_RESOLUTION.day;
    console.log("Resolution : day");
  } else if (diffMinutes > pointThreshold) {
    resolution = DATA_RESOLUTION.hour;
    console.log("Resolution : hour");
  } else {
    resolution = DATA_RESOLUTION.minute;
    console.log("Resolution : minute");
  }

  getTempsInRange_(date_min, date_max, resolution, set_data_fn);
}

/* Given two Date times, get all points within that range (inclusive)
 * Data resolution is either minutes, hours, or days, depending on range
 * such that number of points below pointThreshold 
 * set_data_fn is passed the data returned
 */
function getTempsInRange_(date_min, date_max, resolution, set_data_fn) {
  var timeDelta; // resolution of data in ms

  var d3_callback = function(dataset) {
    if (!dataset) {
      console.log("ERROR, NO DATA", dataset);
    }
    dataset.forEach(function(d) {
      // Hourly
      d.datetime = new Date(d.datetime).getTime();
      d.out_temperature_min = Number(d.out_temperature_min);
      d.out_temperature_mean = Number(d.out_temperature_mean);
      d.out_temperature_max = Number(d.out_temperature_max);
    });

    // convert to format used by highcharts
    // [time, open, max, min, close]
    // for simplicity we'll have open low close high
    var data = dataset.map(function(d) {
      // return [d.datetime, d.out_temperature_mean, d.out_temperature_max, d.out_temperature_min, d.out_temperature_mean];
      return [d.datetime, d.out_temperature_mean];
    });
    data = data.filter(function(d) {
      return !isNaN(d[1]);
    });
    alldata = data;

    // Callback with data
    set_data_fn(data);
  };

  // Used to find closest index in array of date
  var bisectDateLow = d3.bisector(function(d) { return new Date(d.datetime).getTime(); }).left;
  var bisectDateHigh = d3.bisector(function(d) { return new Date(d.datetime).getTime(); }).left;

  if (resolution == DATA_RESOLUTION.day) {
    timeDelta = 1000*60*60*24;
    console.log("Loading daily.");
    // Just use all the daily since it's only 274 values
    d3.csv('../csv/R2015_daily.csv', d3_callback);
  } else if (resolution == DATA_RESOLUTION.hour) {
    timeDelta = 1000*60*60;
    console.log("Loading hourly.");
    d3.csv('../csv/R2015_hourly.csv', function(dataAll) {
      // Gets all hourly data, slice up first
      var min_index = bisectDateLow(dataAll, date_min);
      var max_index = bisectDateHigh(dataAll, date_max);
      // console.log("Sliced to ", min_index, max_index);
      var data = dataAll.slice(min_index, max_index);
      console.log("Returning",data.length,"hourly data points");
      d3_callback(data);
    });
  } else if (resolution == DATA_RESOLUTION.minute) {
    timeDelta = 1000*60;
    console.log("Loading by the minute.");
    loadCombinedMinuteCSVs(date_min, date_max, set_data_fn);
  } else {
    console.log("TODO: RESOLUTION ("+resolution+") NOT EXPECTED/IMPLEMENTED YET");
  }
}

function loadCombinedMinuteCSVs(date_min, date_max, set_data_fn) {
  date_min = new Date(date_min);
  date_max = new Date(date_max);
  var start_filename = "R"+date_min.toString("yyyy_MM_dd")+".csv"
  var end_filename = "R"+date_max.toString("yyyy_MM_dd")+".csv"

  var d3_callback = function(dataset) {
    if (!dataset) {
      console.log("ERROR, NO DATA!", dataset);
    }
    dataset.forEach(function(d) {
      // Hourly
      d.datetime = new Date(d.datetime).getTime();
      d.out_temperature = Number(d.out_temperature);
    });

    // convert to format used by highcharts
    // [time, open, max, min, close]
    // all the same in this case
    var data = dataset.map(function(d) {
      return [d.datetime, d.out_temperature, d.out_temperature, d.out_temperature, d.out_temperature];
    });

    // Slice data
    // Used to find closest index in array of date
    var bisectDateLow = d3.bisector(function(d) { return d[0]; }).left;
    var bisectDateHigh = d3.bisector(function(d) { return d[0]; }).left;
    var min_index = bisectDateLow(data, date_min);
    var max_index = bisectDateHigh(data, date_max);
    data = data.slice(min_index, max_index);

    console.log("Returning",data.length,"minutely data points");
    alldata = data;

    // Callback with data
    set_data_fn(data);
  };


  if (start_filename == end_filename) {
    // 1 day only, no need to merge
    console.log("Loading single day from:", ('../csv/'+start_filename));
    d3.csv('../csv/'+start_filename, d3_callback);
  } else {
    q = queue();
    var sdate = new Date(date_min.getFullYear(), date_min.getMonth(), date_min.getDate()).getTime();
    var edate = new Date(date_max.getFullYear(), date_max.getMonth(), date_max.getDate()).getTime();
    console.log("Loading multiday from",start_filename,"to",end_filename,"...");

    var oneday = 1000 * 60 * 60 * 24;
    if ((edate - sdate)/oneday > 5) {
      console.log("ERROR: Asking to load ", (edate - sdate)/oneday, "days", date_min, " to ", date_max);
    } else
    {
      for (var cdate = sdate; cdate <= edate; cdate += oneday) {
        var currfilename = "../csv/R"+(new Date(cdate)).toString("yyyy_MM_dd")+".csv";
        console.log(" > multiday queue loading", currfilename);
        q.defer(d3.csv, currfilename);
      };
      q.awaitAll(combine);
      function combine(error, allData) {
          if (error) {
              console.log(error);
          }
          dataset = d3.merge(allData);

          // Convert to number format for date and temp
          dataset.forEach(function(d) {
            // Hourly
            d.datetime = new Date(d.datetime).getTime();
            d.out_temperature = Number(d.out_temperature);
          });

          // convert to format used by highcharts
          // [time, open, max, min, close]
          // all the same in this case
          var data = dataset.map(function(d) {
            return [d.datetime, d.out_temperature, d.out_temperature, d.out_temperature, d.out_temperature];
          });

          // Slice data
          // Used to find closest index in array of date
          var bisectDateLow = d3.bisector(function(d) { return d[0]; }).left;
          var bisectDateHigh = d3.bisector(function(d) { return d[0]; }).left;
          var min_index = bisectDateLow(data, date_min);
          var max_index = bisectDateHigh(data, date_max);
          data = data.slice(min_index, max_index);

          console.log("Returning",data.length,"minutely data points");
          alldata = data;

          set_data_fn(data);
      }
    }
  }
}


/**
 * Load new data depending on the selected min and max
 */
function afterSetExtremes(e) {

  var chart = $('#container').highcharts();

  chart.showLoading('Loading data from JSON...');
  getTempsInRange(Math.round(e.min), Math.round(e.max), function(data) {
      chart.series[0].setData(data);
      chart.hideLoading();
    })
}

// Year, month, day, hour, minute, second, ms
var a = new Date(2015,0).getTime();
var b = new Date(2016,0).getTime();
var navigator_data;

// Initialize with entire year
getTempsInRange(a, b, function(data) {
  // Add null to last date
  // data = [].concat(data, [[Date.UTC(2016, 0), null, null, null, null]]);
  // console.log(data);
  // navigator_data = data;
  navigator_data = data.map(function(d) {
    return [d[0], d[1]];
  });
  // Remove NaN values from navigator to show data
  navigator_data = navigator_data.filter(function(d) {
    return !isNaN(d[1]);
  });

  data = data.map(function(d) {
    return [d[0], d[1]];
  });
  data = data.filter(function(d) {
    return !isNaN(d[1]);
  });


  // alldata = navigator_data;
  // data = data.sort(function(a,b) { return a[0] - b[0]; });
  // create the chart
  $('#container').highcharts('StockChart', {
    chart: {
      type: 'area',
      zoomType: 'x'
    },

    navigator: {
      adaptToUpdatedData: false,
      series: {
        data: navigator_data
      },
    },

    scrollbar: {
      liveRedraw: false
    },

    title: {
      text: 'Arduino temperature data for 2015'
    },

    subtitle: {
      text: 'Displaying 400k temperature readings (every minute) in Highcharts Stock via async loading'
    },

    rangeSelector: {
      buttons: [{
        type: 'hour',
        count: 12,
        text: '12h'
      },{
        type: 'day',
        count: 1,
        text: '1d'
      }, {
        type: 'day',
        count: 2,
        text: '2d'
      }, {
        type: 'day',
        count: 7,
        text: '1wk'
      }, {
        type: 'month',
        count: 1,
        text: '1m'
      }, {
        type: 'all',
        text: 'All'
      }],
      inputEnabled: false, // it supports only days
      selected: 5 // all
    },

    xAxis: {
      events: {
        afterSetExtremes: afterSetExtremes
      },
      minRange: 3600 * 24 * 1000 // one day
    },

    yAxis: {
      floor: 0,
    },

    series: [{
      data: data,
      dataGrouping: {
        enabled: false
      }
    }]
  });

});
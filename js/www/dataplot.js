// Generate an SVG bar plot of temperatures with axes

function plotData(dataset)
{
  //Width and height
  var w = 800;
  var h = 400;

  var svg = d3.select("body")
            .append("svg")
            .attr("width", w)
            .attr("height", h);

  var barPadding = 0;
  var padding = 40;
  var padding_h = 80;

  // Scales
  var xScale = d3.time.scale()
                       .domain([d3.min(dataset, function(d){return d.datetime;}),
                                d3.max(dataset, function(d){return d.datetime;})])
                       .range([padding_h,w-padding_h]);

  var yScale = d3.scale.linear()
                       .domain([d3.min(dataset, function(d){return Number(d.out_temperature);}),
                                d3.max(dataset, function(d){return Number(d.out_temperature);})])
                       .rangeRound([h-padding,padding])
                       .clamp(true);
  
  // Bars
  svg.selectAll("rect")
      .data(dataset)
      // .data(dataset, function(d) {return Math.floor((d.datetime-dataset[0].datetime)/(1000*60*60*8))})
      // .data(dataset, function(d) {return Math.floor((d.datetime-dataset[0].datetime)/(1000*60*60*24))})
      .enter()
      .append("rect")
      .attr("x", function(d, i) {return xScale(new Date(d.datetime+"-07:00"));})
      .attr("y", function(d) {
        if (d.out_temperature){
          return yScale(d.out_temperature);
        } else {
          return yScale(0); // NaN = 0
        }
      })
      .attr("width", function(d) {return (w-padding_h*2)/(dataset.length-1);})
      .attr("fill", function(d) {
          return "rgb(0, 0, " + Math.round((d.out_temperature * 155)+100) + ")";
      })
      .attr("height", function(d) {
        if (d.out_temperature){
          return h-padding-yScale(d.out_temperature);
        } else {
          return 0; // NaN = 0
        }
      })
      ;

    // Axes

    var xAxis = d3.svg.axis()
                  .scale(xScale)
                  .orient("bottom")
                  .tickFormat(d3.time.format("%b %d"))
                  .ticks(10);
                  // .tickFormat(d3.time.format("%a %b %d %I:%M %p"))
                  // .ticks(5);

    var yAxis = d3.svg.axis()
                  .scale(yScale)
                  .orient("left");

    svg.append("g")
      .attr("class", "axis")
      .attr("transform", "translate(0," + (h - padding) + ")")
      .call(xAxis);

    svg.append("g")
    .attr("class", "axis")
    .attr("transform", "translate(" + padding_h + ",0)")
    .call(yAxis);

    // Mouse Overlay
    var focus = svg.append("g")
      .attr("class", "focus")
      .style("display", "none");


    focus.append("line")
        .attr("x1", 0)
        .attr("x2", 0)
        .attr("y1", padding)
        .attr("y2", h-padding);
    
    focus.append("circle");

    focus.append("text")
        .attr("x", 9)
        .attr("dy", ".35em")
        .attr("transform","translate(0,"+(padding-10)+")")
        .attr("text-anchor", "middle");
    
    svg.append("rect")
      .attr("class", "overlay")
      .attr("width", w)
      .attr("height", h)
      .on("mouseover", function() { focus.style("display", null); })
      .on("mouseout", function() { focus.style("display", "none"); })
      .on("mousemove", mousemove);

  bisectDate = d3.bisector(function(d) { return d.datetime; }).left;

  var datetimeFormatter = d3.time.format("%I:%M %p");

  function mousemove() {
    // clamp range since we can go out of bounds
    var x0 = xScale.invert(d3.mouse(this)[0]);
        i = Math.min(dataset.length-1, Math.max(1,bisectDate(dataset, x0))),
        d0 = dataset[i - 1],
        d1 = dataset[i],
        d = x0 - d0.datetime > d1.datetime - x0 ? d1 : d0;
    focus.attr("transform",
      "translate(" + xScale(d.datetime) + "," + 0 + ")");
    if (d.out_temperature)
    {
      focus.select("circle")
      .attr("cy",yScale(d.out_temperature))
    } else {
      focus.select("circle")
      .attr("cy",yScale(0))
    }
    focus.select("text")      
      .text(datetimeFormatter(d.datetime) + " - " + d.out_temperature+"°C");
  }
}

// Load data from CSV
var alldata;
// d3.csv('../data/R2015_07_13.csv', function(dataset) {
//   // Convert timestamps to Dates
//   dataset.forEach(function(d) {
//     d.datetime = new Date(d.datetime+"-07:00")
//     d.out_temperature = Number(d.out_temperature); //max/min fail secretly otherwise
//   });
//   alldata = dataset;

//   plotData(dataset);
// });

d3.csv('../data/R2015_daily.csv', function(dataset) {
// d3.csv('../data/R2015_hourly.csv', function(dataset) {
  // dataset = dataset.slice(0,1000);
  
  // Convert timestamps to Dates
  dataset.forEach(function(d) {
    // d.datetime = new Date(d.datetime+"-07:00")

    // Hourly
    d.datetime = new Date(d.datetime+"-08:00")
    d.out_temperature = Number(d.out_temperature_mean);
  });
  alldata = dataset;

  plotData(dataset);
});


// queue()
//     .defer(d3.csv, "../data/R2015_06_12.csv")
//     .defer(d3.csv, "../data/R2015_06_13.csv")
//     .defer(d3.csv, "../data/R2015_06_14.csv")
//     .await(combine);

// function combine(error, big_data_1, big_data_2, big_data_3) {
//     if (error) {
//         console.log(error);
//     }
//     dataset = d3.merge([big_data_1, big_data_2, big_data_3]);
//     plotData(dataset);
// }
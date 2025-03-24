let chart = null;
let candlestickSeries = null;
let maSeries = null;

// Initialize the chart
function initChart() {
    console.log('Initializing chart...');
    console.log('LightweightCharts object:', LightweightCharts);
    
    const chartContainer = document.getElementById('chartContainer');
    if (!chartContainer) {
        console.error('Chart container not found');
        return;
    }
    
    try {
        // Create the chart
        chart = LightweightCharts.createChart(chartContainer, {
            layout: {
                background: { color: 'transparent' },
                textColor: '#ffffff',
            },
            grid: {
                vertLines: { color: 'rgba(255, 255, 255, 0.1)' },
                horzLines: { color: 'rgba(255, 255, 255, 0.1)' },
            },
            width: chartContainer.clientWidth,
            height: 500,
            timeScale: {
                timeVisible: true,
                secondsVisible: false,
                borderColor: 'rgba(255, 255, 255, 0.2)',
                fixLeftEdge: true,
                fixRightEdge: true,
                lockVisibleTimeRangeOnResize: true,
                rightBarStaysOnScroll: true,
                borderVisible: true,
                visible: true,
                tickMarkFormatter: (time) => {
                    const date = new Date(time * 1000);
                    return date.toLocaleDateString();
                }
            },
        });

        console.log('Chart created successfully');
        console.log('Chart object:', chart);

        // Create candlestick series
        candlestickSeries = chart.addCandlestickSeries({
            upColor: '#00ff9d',
            downColor: '#ff4444',
            borderVisible: false,
            wickUpColor: '#00ff9d',
            wickDownColor: '#ff4444',
        });

        // Create moving average series
        maSeries = chart.addLineSeries({
            color: '#ffffff',
            lineWidth: 2,
            lastValueVisible: false,
        });

        console.log('Series created successfully');
    } catch (error) {
        console.error('Error creating chart:', error);
    }

    // Handle window resize
    window.addEventListener('resize', () => {
        if (chart) {
            chart.applyOptions({
                width: chartContainer.clientWidth,
            });
        }
    });
}

// Calculate 20-day moving average
function calculateMA(data) {
    console.log('Calculating moving average...');
    const maData = [];
    for (let i = 0; i < data.length; i++) {
        if (i < 19) {
            maData.push({ time: data[i].time, value: null });
            continue;
        }
        
        let sum = 0;
        for (let j = 0; j < 20; j++) {
            sum += data[i - j].close;
        }
        maData.push({
            time: data[i].time,
            value: sum / 20
        });
    }
    return maData;
}

// Fetch stock data from Yahoo Finance
async function fetchStockData(symbol) {
    const loading = document.getElementById('loading');
    const chartContainer = document.getElementById('chartContainer');
    
    // Show loading state
    loading.style.display = 'block';
    chartContainer.style.opacity = '0.5';
    
    console.log(`Fetching data for ${symbol}...`);

    try {
        const response = await fetch(`/api/stock/${symbol}`);
        if (!response.ok) {
            const errorData = await response.json();
            throw new Error(errorData.details || 'Failed to fetch stock data');
        }
        
        const data = await response.json();
        console.log(`Received ${data.length} data points`);
        
        if (data.length === 0) {
            throw new Error('No data received for the selected stock');
        }
        
        // Format data for the chart
        const chartData = data.map(item => ({
            time: item.date.split('T')[0], // Ensure we only use the date part
            open: parseFloat(item.open),
            high: parseFloat(item.high),
            low: parseFloat(item.low),
            close: parseFloat(item.close)
        }));

        console.log('Formatted chart data:', chartData.slice(0, 2));

        // Update candlestick series
        candlestickSeries.setData(chartData);
        console.log('Candlestick data set successfully');

        // Calculate and update moving average
        const maData = calculateMA(chartData);
        maSeries.setData(maData);
        console.log('Moving average data set successfully');

        // Fit content and set visible range
        chart.timeScale().fitContent();
        chart.timeScale().setVisibleRange({
            from: chartData[0].time,
            to: chartData[chartData.length - 1].time
        });
        console.log('Chart content fitted and range set');
        
        // Reset chart container opacity
        chartContainer.style.opacity = '1';
    } catch (error) {
        console.error('Error fetching stock data:', error);
        alert(`Error: ${error.message}`);
        // Clear the chart on error
        candlestickSeries.setData([]);
        maSeries.setData([]);
    } finally {
        loading.style.display = 'none';
    }
}

// Initialize the chart when the page loads
document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM Content Loaded');
    initChart();

    // Add event listener for stock selection
    const tickerSelect = document.getElementById('tickerSelect');
    tickerSelect.addEventListener('change', (e) => {
        const symbol = e.target.value;
        console.log('Stock selected:', symbol);
        if (symbol) {
            fetchStockData(symbol);
        }
    });
}); 
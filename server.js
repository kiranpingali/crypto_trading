const express = require('express');
const path = require('path');
const yahooFinance = require('yahoo-finance2').default;
const app = express();
const port = 3000;

// Serve static files from the public directory
app.use(express.static('public'));

// Serve the main page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Serve the stocks page
app.get('/stocks', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'stocks.html'));
});

// API endpoint for stock data
app.get('/api/stock/:symbol', async (req, res) => {
    try {
        const { symbol } = req.params;
        console.log(`Fetching data for symbol: ${symbol}`);
        
        const oneYearAgo = new Date();
        oneYearAgo.setFullYear(oneYearAgo.getFullYear() - 1);

        const result = await yahooFinance.historical(symbol, {
            period1: oneYearAgo,
            period2: new Date(),
            interval: '1d'
        });

        console.log(`Received ${result.length} data points for ${symbol}`);

        // Format the data
        const formattedData = result.map(item => ({
            date: item.date.toISOString().split('T')[0],
            open: item.open,
            high: item.high,
            low: item.low,
            close: item.close
        }));

        res.json(formattedData);
    } catch (error) {
        console.error('Error fetching stock data:', error);
        res.status(500).json({ 
            error: 'Failed to fetch stock data',
            details: error.message 
        });
    }
});

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
}); 
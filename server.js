const express = require('express');
const path = require('path');
const yahooFinance = require('yahoo-finance2').default;
const app = express();
const port = 3000;

// Enable JSON parsing for request body
app.use(express.json());

// Serve static files from the public directory
app.use(express.static('public'));

// In-memory order book
const orderBook = {
    orders: [],
    lastPrices: new Map()
};

// Serve the main page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Serve the stocks page
app.get('/stocks', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'stocks.html'));
});

// API endpoint for NASDAQ 100 constituents
app.get('/api/nasdaq100', async (req, res) => {
    try {
        // Use a predefined list of major tech stocks
        const techStocks = [
            { symbol: 'AAPL', name: 'Apple Inc.' },
            { symbol: 'MSFT', name: 'Microsoft Corporation' },
            { symbol: 'AMZN', name: 'Amazon.com Inc.' },
            { symbol: 'GOOGL', name: 'Alphabet Inc. (Google)' },
            { symbol: 'META', name: 'Meta Platforms Inc.' },
            { symbol: 'NVDA', name: 'NVIDIA Corporation' },
            { symbol: 'TSLA', name: 'Tesla Inc.' },
            { symbol: 'AMD', name: 'Advanced Micro Devices' },
            { symbol: 'INTC', name: 'Intel Corporation' },
            { symbol: 'CSCO', name: 'Cisco Systems Inc.' },
            { symbol: 'ADBE', name: 'Adobe Inc.' },
            { symbol: 'PYPL', name: 'PayPal Holdings Inc.' },
            { symbol: 'CMCSA', name: 'Comcast Corporation' },
            { symbol: 'PEP', name: 'PepsiCo Inc.' },
            { symbol: 'COST', name: 'Costco Wholesale Corporation' },
            { symbol: 'TMUS', name: 'T-Mobile US Inc.' },
            { symbol: 'QCOM', name: 'QUALCOMM Inc.' },
            { symbol: 'GILD', name: 'Gilead Sciences Inc.' },
            { symbol: 'MDLZ', name: 'Mondelez International Inc.' },
            { symbol: 'ADP', name: 'Automatic Data Processing Inc.' }
        ];

        console.log(`Returning ${techStocks.length} tech stocks`);
        res.json(techStocks);
    } catch (error) {
        console.error('Error in NASDAQ 100 endpoint:', error);
        res.status(500).json({ 
            error: 'Failed to fetch stock list',
            details: error.message 
        });
    }
});

// API endpoint for stock data
app.get('/api/stock/:symbol', async (req, res) => {
    try {
        const { symbol } = req.params;
        const result = await yahooFinance.historical(symbol, {
            period1: '2023-01-01',
            period2: new Date(),
            interval: '1d'
        });

        console.log(`Fetched ${result.length} data points for ${symbol}`);
        res.json(result);
    } catch (error) {
        console.error('Error fetching stock data:', error);
        res.status(500).json({ 
            error: 'Failed to fetch stock data',
            details: error.message 
        });
    }
});

// API endpoint for order submission
app.post('/api/order', async (req, res) => {
    try {
        const { symbol, quantity, type, timestamp } = req.body;
        
        // Validate order
        if (!symbol || !quantity || !type) {
            return res.status(400).json({ error: 'Invalid order parameters' });
        }

        // Get current price
        const quote = await yahooFinance.quote(symbol);
        const price = quote.regularMarketPrice;
        
        // Calculate total
        const total = price * quantity;

        // Store order
        const order = {
            id: Date.now().toString(),
            symbol,
            quantity,
            type,
            price,
            total,
            timestamp,
            status: 'EXECUTED'
        };

        orderBook.orders.push(order);
        orderBook.lastPrices.set(symbol, price);

        // Return execution details
        res.json({
            orderId: order.id,
            symbol,
            quantity,
            type,
            price,
            total,
            timestamp: order.timestamp,
            status: order.status
        });

    } catch (error) {
        console.error('Error processing order:', error);
        res.status(500).json({ 
            error: 'Failed to process order',
            details: error.message 
        });
    }
});

// API endpoint for order history
app.get('/api/orders', (req, res) => {
    res.json(orderBook.orders);
});

// API endpoint for last price
app.get('/api/price/:symbol', (req, res) => {
    const { symbol } = req.params;
    const lastPrice = orderBook.lastPrices.get(symbol);
    
    if (!lastPrice) {
        return res.status(404).json({ error: 'No price data available' });
    }
    
    res.json({ symbol, price: lastPrice });
});

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
}); 
const express = require('express');
const cors = require('cors');
const yahooFinance = require('yahoo-finance2').default;
const app = express();
const port = 3001;

// Enable CORS for the stocks application
app.use(cors());
app.use(express.json());

// In-memory order book (simplified)
const orderBook = {
    orders: [],
    lastPrices: new Map()
};

// Function to get current price from Yahoo Finance
async function getCurrentPrice(symbol) {
    try {
        const quote = await yahooFinance.quote(symbol);
        return quote.regularMarketPrice;
    } catch (error) {
        console.error(`Error fetching price for ${symbol}:`, error);
        throw new Error('Failed to fetch current price');
    }
}

// Process order endpoint
app.post('/api/order', async (req, res) => {
    try {
        const { symbol, quantity, type, timestamp } = req.body;
        
        // Validate order
        if (!symbol || !quantity || !type) {
            return res.status(400).json({ error: 'Invalid order parameters' });
        }

        // Get current price
        const price = await getCurrentPrice(symbol);
        
        // Calculate total
        const total = price * quantity;

        // Store order in order book
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

// Get order history endpoint
app.get('/api/orders', (req, res) => {
    res.json(orderBook.orders);
});

// Get last price endpoint
app.get('/api/price/:symbol', (req, res) => {
    const { symbol } = req.params;
    const lastPrice = orderBook.lastPrices.get(symbol);
    
    if (!lastPrice) {
        return res.status(404).json({ error: 'No price data available' });
    }
    
    res.json({ symbol, price: lastPrice });
});

app.listen(port, () => {
    console.log(`Exchange Simulator running at http://localhost:${port}`);
}); 
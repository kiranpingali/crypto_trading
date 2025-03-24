function animateText() {
    const title = document.querySelector('.title');
    const subtitle = document.querySelector('.subtitle');
    
    // Add pulse animation to title
    title.style.animation = 'pulse 0.5s ease';
    
    // Change subtitle text with a fade effect
    subtitle.style.opacity = '0';
    setTimeout(() => {
        subtitle.textContent = 'Thanks for clicking! 🚀';
        subtitle.style.opacity = '1';
    }, 200);
    
    // Remove animation after it completes
    setTimeout(() => {
        title.style.animation = '';
    }, 500);
} 
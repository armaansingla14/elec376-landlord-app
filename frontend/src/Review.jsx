import React from 'react'
import { Link } from 'react-router-dom'

export default function Review() {
  const pageStyle = {
    minHeight: '100vh',
    background: 'linear-gradient(135deg, rgba(96, 165, 250, 0.8) 0%, rgba(59, 130, 246, 0.8) 100%)',
    padding: '120px 24px 40px',
  }

  const contentStyle = {
    maxWidth: '800px',
    margin: '0 auto',
    backgroundColor: 'rgba(255, 255, 255, 0.15)',
    backdropFilter: 'blur(20px)',
    borderRadius: '24px',
    padding: '32px',
    border: '1px solid rgba(255, 255, 255, 0.3)',
    boxShadow: '0 8px 32px rgba(0, 0, 0, 0.1)',
  }

  const titleStyle = {
    fontSize: '2.5rem',
    fontWeight: '800',
    color: '#ffffff',
    textAlign: 'center',
    marginBottom: '32px',
    textShadow: '0 2px 4px rgba(0, 0, 0, 0.1)'
  }

  return (
    <div style={pageStyle}>
      <div style={contentStyle}>
        <h1 style={titleStyle}>Write a Review</h1>
        {/* We'll add the review form here in the next step */}
      </div>
    </div>
  )
}
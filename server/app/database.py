from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession
from sqlalchemy.orm import sessionmaker
from app.config import settings
from app.models import Base

# Create async engine
engine = create_async_engine(
    settings.database_url,
    echo=True,
    future=True
)

# Create async session factory
async_session_maker = sessionmaker(
    engine, class_=AsyncSession, expire_on_commit=False
)


async def init_db():
    """Initialize database tables."""
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)


async def get_session() -> AsyncSession:
    """Dependency for getting async database sessions."""
    async with async_session_maker() as session:
        yield session

(define (problem ijrr_15)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street8 street13 - street
		building1 building2 building4 building5 - building
	)
	(:init
		(atLocation survivor0 street13)
		(atLocation survivor1 building4)
		(onFire building1)
		(onFire building2)
		(underRubble street8)
		(underRubble building5)
		(needsRepair building1)
		(needsRepair building2)
		(needsRepair building5)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street8))
		(not (needsRepair building1))
		(not (needsRepair building2))
		(not (needsRepair building5))
	))
	(:metric minimize total-time)
)
